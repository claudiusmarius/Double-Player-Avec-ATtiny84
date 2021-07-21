  // ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  // Commande de 2DFPlayer Mini à partir d'un ATtny84 - Horloge interne 8MHz (ne pas oublier de graver la séquence)
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Fonction ACCELERATION <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  // La présence de parasites sonores a souvent été mentionnée par les utilisateurs de DFPlayer Mini, ce montage et le code, les suppriment.
  // Attention le câblage doit être très soigné pour les Players, lorsque l'ampli est utilisé et que le Player est monté sur breadboard, un condensateur de 330µF et un autre de 470nF 
  // sont montés sur l'alimentation au plus prés du Player, les liaisons doivent être les plus courtes possibles, notamment par l'emploi de pinheaders strapps, les Dupont doivent   
  // être réduits au maximum.
  // Une alimentation dédiée est vivement recommandée pour alimenter les players, ne surtout pas passer par l'alimentation en provenance de l'USB.
  // Bien sur, ne pas oublier de relier les masses de la partie MCU et de la puissance (Players).
  // RP2 servait à simuler un régime moteur par augmentation volume sonore du moteur, cette solution fonctionnait très bien avec un ATtiny85, cependant, les résultats sont beaucoup 
  // moins concluants avec un ATtiny84 utilisant 2 liaisons RX/TX, RP2 (potPin) a néanmoins gardé en vue d'améliorations futures (temps de retard sur ATtiny84)
  // Les ports TX1 et TX2 sont ouverts que si nécessaire (on les referme dés que l'on a plus besoin de faire transiter des informations, c'est une parade pour limiter les cliquetis
  // La sortie 6 (MosfetGate) est dédiée à une future évolution.
  // HP1 et HP2 sont des petits speakers de 4 ou 8 ohms récupérés sur un vieux ordinateur portable.
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Claude DUFOURMONT +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //                                                                         
  // ##################################################################### claude.dufourmont@laposte.net #############################################################################
  //
  //                                                              ############################################################
  //                                                                      youtube.com/channel/UCvr9eb05lJow6N7m3SKqvNw  
  //                                                              ############################################################
  
  
  #include "Arduino.h"
  #include <SoftwareSerial.h>
  #include "DFRobotDFPlayerMini.h"
  
  #define TX1 4                                                         // Vers RX DFPlayer Mini 01
    
  #define TX2 0                                                         // Vers RX DFPlayer Mini 02
  
  #define potPin A2                                                     // Vers curseur RP2
  
  #define BPStartStop 10                                                // KEY3 ici ce BP ne sert qu'à lancer le cycle "arrêt moteur"
  #define MosfetGate 6                                                  // Pour l'instant c'est la LED D1 - Ce port est destiné à une évolution future 1seul bouton Start/Stop                 
  
  #define BPKlaxon 7                                                    // KEY1
  #define BPAirPurge 8                                                  // KEY4, relaché d'air périodique selon la période "interval"

  // NOTE IMPORTANTE : Dans cette version KEY2 est câblé entre le Reset et le zéro volt de manière à relancer un cycle de démarrage.

   
  // ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
     
   SoftwareSerial mySoftwareSerial_01 (5,4);                                          // RX1, TX1
   SoftwareSerial mySoftwareSerial_02 (1,0);                                          // RX2, TX2
  
  // Réaffectation des RX/TX pour ne pas avoir de problème lors du téléversement
  // Seuls les port TX sont réellement utilisés, les DFPlayer Mini, ne retournant rien dans cette configuration
  // Les 1,2K branchées en série sur les port TX n'ont pas été installées car inefficaces, j'ai ajouté des zéners avec résistance pour écrêter les signaux TX aux valeurs de traitement par DFPlayer
  // par DFPlayer Mini
  // ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  DFRobotDFPlayerMini myDFPlayer01; 
  byte volumeLevel_01 = 0;                                                            // variable du DFPlayer01 pour le contrôle de volume (0 à 30)
  bool StateBPStartStop = 0;                                                          // état du BP Start ou Stop géré par le DFPlayer01
  bool EnMarche = 0;
  
  DFRobotDFPlayerMini myDFPlayer02;
  byte volumeLevel_02 = 0;
  bool StateBPKlaxon = 0;                                                             // état du BP Klaxon géré par le DFPlayer02
  bool StateBPAirPurge = 0;                                                           // état du BP AirPurge géré par le DFPlayer02
   
  const long interval = 15000;                                                        // Période PurgeAir                                                                         
  unsigned long previousMillis = 0;
  
  void setup()
  {
  
  EnMarche = 1;                                                                        // Dés que l'ATtiny84 est démarré, le moteur est déclaré en marche
  pinMode (TX1, OUTPUT);                                                               // TX1 est déclaré ouverte
  mySoftwareSerial_01.begin(9600);
  myDFPlayer01.begin (mySoftwareSerial_01);
  
  pinMode (BPStartStop, INPUT_PULLUP);                                                  // Déclaration des E/S
  pinMode (MosfetGate, OUTPUT);
  
  volumeLevel_01 = 30;                                                                  // Niveau sonore pour jouer le son du démarreur
 
  myDFPlayer01.play(7);       //7                                                       // Jouer la piste 7 du DFPlayer01 une seule fois (son du démarreur)                                     
  
  delay (1240);                                                                         // Le délais correspond à une attente équivalente à la durée démarreur (avant lancement de la prochaine commande)
    
  myDFPlayer01.loop(6);      //6                                                        // Jouer en bouccle infinie la piste 6 du DFPlayer01                                    
  delay (5); 
  pinMode (TX1, INPUT_PULLUP);                                                          // Mise en haute impédance du port de liaison série TX1
  digitalWrite (MosfetGate, LOW); 
  EnMarche = 1; 
  
  mySoftwareSerial_02.begin(9600);
  myDFPlayer02.begin (mySoftwareSerial_02);
  pinMode (BPKlaxon, INPUT_PULLUP);
  pinMode (BPAirPurge, INPUT_PULLUP);
  }
 
  void loop()
  {
  
  delay(5);
  pinMode (TX1, OUTPUT);                                                                  // Le port TX1 est ouvert (positionné en sortie)
  volumeLevel_01 = map(analogRead(potPin), 0, 1023, 15, 30);                              // Calibrage du niveau sonore en fonction de la valeur du potentiomètre
  myDFPlayer01.volume(volumeLevel_01);  
  delay(5);
  pinMode (TX1, INPUT_PULLUP);                                                            // Mise en haute impédance du port de liaison série TX1
  unsigned long currentMillis = millis();                                                 // Mémorisation de la valeur courante du temps écoulé à l'instant T
  StateBPKlaxon =digitalRead (BPKlaxon);
  if (currentMillis - previousMillis >= interval && EnMarche && StateBPKlaxon ==1)        // Test de dépassement de l'intervalle avec la condition moteur en marche
  {
  previousMillis = currentMillis;
  pinMode (TX2, OUTPUT);
  myDFPlayer02.volume(29);                                                                 // Volume sur DFPlayer02
  myDFPlayer02.play(3);                                                                    // Jouer la piste 3 sur DFPlayer02 (échappement d'air)
  pinMode (TX2, INPUT_PULLUP);                                                             // Le port TX2 est ouvert (positionné en sortie)
  delay (5);
  }
    
  StateBPStartStop =digitalRead (BPStartStop );                                                 
  
  if (StateBPStartStop == LOW && EnMarche == 0)                                            // Conditions d'action suite appui du BP Start Stop
  {
  digitalWrite (MosfetGate, HIGH);
  delay(5);
  }

  if (StateBPStartStop == LOW && EnMarche == 1)
  {
  digitalWrite (MosfetGate, LOW);
  delay(5);
  pinMode (TX1, OUTPUT);                                                                    // Ouverture de la liaison série pour passer des commandes sur DFPlayer01
  myDFPlayer01.volume(30);
  delay(5);
  //myDFPlayer01.disableLoopAll(); // Ne pas laisser cette ligne évite un léger blanc sonore indésirable// Comme une demande de Stop est interprétée, désactivation de toutes les pistes en cours sur le DFPlayer01
  myDFPlayer01.play(11);                                                                    // Jouer la piste 11 du DFPlayer01 (son de l'arrêt moteur)
  pinMode (TX1, INPUT_PULLUP);
  delay (5);                                                                                
  EnMarche = 0;                                                                             // L'état du moteur est déclaré à l'arrêt 
  }
 
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Actionnement Klaxon<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  StateBPKlaxon =digitalRead (BPKlaxon);                                                    // Actualisation de l'état Klaxon
  if (StateBPKlaxon == LOW)                                                                 // Si l'état Klaxon est à 1
  { 
  pinMode (TX2, OUTPUT);                                                                    // Le port TX2 est ouvert (positionné en sortie)
  myDFPlayer02.volume(30);                                                                  // Volume à fond sur DFPlayer02
  myDFPlayer02.play(8);                                                                     // Jouer la piste 1 sur DFPlayer02
  pinMode (TX2, INPUT_PULLUP);
  delay (5);
  }
  
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Actionnement AirPurge<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  StateBPAirPurge =digitalRead (BPAirPurge);                                                // Actualisation de l'état Klaxon
  if (StateBPAirPurge == LOW)                                                               // Si l'état Klaxon est à 1
  { 
  pinMode (TX2, OUTPUT);
  myDFPlayer02.volume(30);                                                                  // Volume à fond sur DFPlayer02
  myDFPlayer02.play(3);                                                                     // Jouer la piste 1 sur DFPlayer02
  pinMode (TX2, INPUT_PULLUP);
  delay (5);
  }
  }
  

  

  

  

 
 
