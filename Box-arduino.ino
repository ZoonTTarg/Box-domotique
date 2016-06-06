#include <SPI.h>
#include <Ethernet.h>
#include <VirtualWire.h> // inclusion de la librairie VirtualWire
#include <string.h>

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 200); // IP address, may need to change depending on network
EthernetServer server(80);  // create a server at port 80

String HTTP_req;          // stores the HTTP request
boolean LED_status = 0;   // state of LED, off by default
const char *PIECE_BUREAU = "D01"; //PIECE_BUREAU
const char *CODE_ACTION_voletouverture = "A0001";
const char *CODE_ACTION_voletfermeture = "A0002";
const char *CODE_ACTION_voletstop = "A0003";

const char *CODE_ACTION = "codeAction";
const char *CODE_APPAREIL = "codeAppareil";

const char *SEPARATEUR = "|";
const char *FINDELIGNE = "\0";
char message[100];

void setup()
{
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
    Serial.begin(9600);       // for diagnostics
    vw_set_tx_pin(2);
    vw_set_rx_pin(3);
    vw_rx_start();
    //pinMode(2, OUTPUT);       // LED on pin 2
    //pinMode(3, OUTPUT);       // LED on pin 3
    vw_setup(2000); // initialisation de la librairie VirtualWire à 2000 bauds (note: je n'utilise pas la broche PTT)
    Serial.println("Initialisation terminee");
}

void loop()
{
    EthernetClient client = server.available();  // try to get client

    if (client) {  // got client?
      Serial.println("Client detecte");
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                HTTP_req += c;  // save the HTTP request 1 char at a time
                 // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n') {
                    Serial.println("La requête est: "+HTTP_req);
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    //client.println("Content-Type: application/json");
                    
                    client.println("Connection: close");
                    client.println();
                    
                    // send web page
                    //client.println("<!DOCTYPE html>");
                    //client.println("<html>");
                    //client.println("<head>");
                    //client.println("<title>Arduino LED Control</title>");
                    //client.println("<link rel=\"icon\" href=\"data:;base64,iVBORw0KGgo=\">");
                    //client.println("</head>");
                    //client.println("<body>");
                    //Passage du volet en montée
                    if(HTTP_req.indexOf(CODE_ACTION_voletouverture) >-1) //Ouverture
                    {
                      Serial.println("Ouverture - HTTP_req:"+HTTP_req);
                      //client.println("<h1>Ouverture</h1>");
                      client.println("\"success\":\"1\"");
                      memset (message, 0, sizeof (message));
                      strcat(message, PIECE_BUREAU);
                      strcat(message, SEPARATEUR);
                      strcat(message, CODE_ACTION_voletouverture);
                      strcat(message, FINDELIGNE);
                      
                      Serial.println("Envoi ouverture"); // On signale le début de l'envoi
                      vw_send((uint8_t *)message, strlen(message)); // On envoie le message 
                      vw_wait_tx(); // On attend la fin de l'envoi
                      Serial.println("Ouverture sélectionnée"); // On signal la fin de l'envoi 
                      delay(10);
                    }
                    //Passage du volet en descente
                    else if(HTTP_req.indexOf(CODE_ACTION_voletfermeture) >-1) //Fermeture
                    {
                      Serial.println("Fermeture - HTTP_req:"+HTTP_req);
                      //client.println("<h1>Fermeture</h1>");
                      client.println("\"success\":\"1\"");
                      memset (message, 0, sizeof (message));
                      strcat(message, PIECE_BUREAU);
                      strcat(message, SEPARATEUR);
                      strcat(message, CODE_ACTION_voletfermeture);
                      strcat(message, FINDELIGNE);
                                            
                      Serial.println("Envoi fermeture"); // On signale le début de l'envoi
                      vw_send((uint8_t *)message, strlen(message)); // On envoie le message 
                      vw_wait_tx(); // On attend la fin de l'envoi
                      Serial.println(message); // On signal la fin de l'envoi
                      delay(10);
                    }
                    //Passage du volet au neutre
                    else if(HTTP_req.indexOf(CODE_ACTION_voletstop) >-1)//On stoppe le courant
                    {
                      memset (message, 0, sizeof (message));
                      strcat(message, PIECE_BUREAU);
                      strcat(message, SEPARATEUR);
                      strcat(message, CODE_ACTION_voletstop);
                      strcat(message, FINDELIGNE);
                      //client.println("<h1>Stop</h1>");
                      client.println("\"success\":\"1\"");
                      Serial.println("Envoi courant OFF"); // On signale le début de l'envoi
                      vw_send((uint8_t *)message, strlen(message)); // On envoie le message 
                      vw_wait_tx(); // On attend la fin de l'envoi
                      Serial.println(message); // On signal la fin de l'envoi
                      delay(10);
                    }
                    //Si j'ai codé comme une merde:
                    else
                    {
                      Serial.print("L'action ");
                      Serial.print(HTTP_req);
                      Serial.println(" n'est pas reconnue par le serveur.");
                      client.println("\"success\":\"0\"");
                      //client.println("<h1>Tu as codé comme une merde</h1>");
                    }
                    //client.println("</body>");
                    //client.println("</html>");
                    
                    HTTP_req = "";    // finished with request, empty string
                    break;
                }
              
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}
