#include <Arduino.h>
#include <TFT_eSPI.h>
#include "cards.h"
#include "rm67162.h"
#include "Latin_Hiragana_24.h"
#include "NotoSansBold15.h"
#include "NotoSansMonoSCB20.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

#define up 21
#define down 0
int buttons[5]={46,45,44,43,42};
bool debB[5]={0,0,0,0,0};
bool deb = 0;
bool deb2 = 0;

int fase=0;
String win="";
int tmpHand[5];
int deck[52];
int hand[5];
bool held[5] = {0,0,0,0,0};
int handRank=0;

int score=20;
int bet=2;
int deal=0;
    
int x[7]={113,253,253,253,393,393,393};
int y[7]={56,0,28,56,0,28,56};
String txt[7]={"Pair","Two Pairs","3 of a Kind","Straight","Flush","Full House","4 of a kind"};
int multi[7]={1,3,4,8,10,14,26};

unsigned long timePassed=0;
unsigned long tempPassed=0;
unsigned long timePassed2=0;
unsigned long tempPassed2=0;
int period=250;
bool started=0;
bool started2=0;


// Function to shuffle an array (Fisher-Yates algorithm)
void shuffle(int *array, int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = random(i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}


int flush(int *hand) {
    int suit = hand[0] / 13;
    for (int i = 1; i < 5; i++) {
        if (hand[i] / 13 != suit) {
            return 0; // Not a flush
        }
    }
    return 1; // Flush
}

// Function to check for a straight
int straight(int *hand) {
    // Sort the hand
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4 - i; j++) {
            if (hand[j] % 13 > hand[j + 1] % 13) {
                int temp = hand[j];
                hand[j] = hand[j + 1];
                hand[j + 1] = temp;
            }
        }
    }

    // Check for a straight
    for (int i = 0; i < 4; i++) {
        if (hand[i] % 13 + 1 != hand[i + 1] % 13) {
            return 0; // Not a straight
        }
    }
    return 1; // Straight
}
// Function to evaluate a poker hand and determine its rank
int evaluateHand(int *hand) {
    // Define arrays for card ranks and suits
    int ranks[13] = {0}; // Initialize rank counters to 0

    // Count the number of each rank in the hand
    for (int i = 0; i < 5; i++) {
        int rank = hand[i] % 13;
        ranks[rank]++;
    }

    int pairs = 0;
    int threeOfAKind = 0;
    int fourOfAKind = 0;

    // Check for pairs, three of a kind, and four of a kind
    for (int i = 0; i < 13; i++) {
        if (ranks[i] == 2) {
            pairs++;
        } else if (ranks[i] == 3) {
            threeOfAKind++;
        } else if (ranks[i] == 4) {
            fourOfAKind++;
        }
    }

    if (fourOfAKind == 1) {
        return 8; // Four of a kind
    } else if (threeOfAKind == 1 && pairs == 1) {
        return 7; // Full House
    } else if (flush(hand)) {
        return 6; // Flush
    } else if (straight(hand)) {
        return 5; // Straight
    } else if (threeOfAKind == 1) {
        return 4; // Three of a kind
    } else if (pairs == 2) {
        return 3; // Two pairs
    } else if (pairs == 1) {
        return 2; // One pair
    }

    return 1; // High card (default)
}



void printHand()
{
     Serial.println("Your hand:");
    for (int i = 0; i < 5; i++) {
        int card = hand[i];
        int rank = card % 13;
        int suit = card / 13;

        // Define arrays for card ranks and suits
        char *ranks[] = {"Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King", "Ace"};
        char *suits[] = {"Clubs", "Diamonds", "Hearts", "Spades"};

        Serial.println(card);
        Serial.print(ranks[rank]);
        Serial.print(" of ");
        Serial.println(suits[suit]);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void draw()
  {
      sprite.fillSprite(TFT_BLACK);
      sprite.setTextDatum(0);

    sprite.fillRect(9,0,98,28,TFT_BLUE);
    sprite.setTextColor(TFT_RED,TFT_BLUE);
    sprite.loadFont(Latin_Hiragana_24);
    sprite.drawString("POKER",14,4);
    sprite.setTextColor(TFT_YELLOW,TFT_BLACK);
    sprite.drawString(String(score),9,54);
    sprite.fillRect(9,76,98,2,0x6B4D);
    sprite.unloadFont();


    sprite.setTextColor(TFT_WHITE,TFT_BLACK);
    sprite.loadFont(NotoSansMonoSCB20);
    sprite.drawString("CREDIT:",9,32);


    sprite.fillRect(113,0,136,50,TFT_YELLOW);
    sprite.fillRect(120,24,84,3,TFT_BLACK);
    sprite.setTextColor(TFT_BLACK,TFT_YELLOW);

    sprite.drawString("BET:  "+String(bet),120,3);
    sprite.drawString("DEAL:  "+String(deal),120,29);

    sprite.unloadFont();
    sprite.loadFont(NotoSansBold15);
    
    sprite.setTextColor(TFT_WHITE,0x01C7);
    sprite.setTextDatum(4);
    for(int i=0;i<7;i++){
    sprite.fillRect(x[i],y[i],136,22,0x0AAB);
    sprite.drawString(txt[i]+" "+String(bet*multi[i]),x[i]+70,y[i]+11);
    }

    if(fase==5 && handRank>1){
    sprite.drawRect(x[handRank-2]+1,y[handRank-2]+1,134,20,TFT_RED);
    sprite.drawRect(x[handRank-2],y[handRank-2],136,22,TFT_RED);
    }
    
    
    for(int i=0;i<5;i++){
    sprite.pushImage(9+(i*104),90,96,134,card[hand[i]]);
   // sprite.drawRoundRect(9+(i*104),84,96,134,8,TFT_BLACK);
    
    if(fase==2){
    //sprite.drawString(String(held[i]),9+48+(i*104),230);
    if(held[i]==1) sprite.fillRect(9+(i*104),234,96,6,TFT_ORANGE);
    else sprite.fillRect(9+(i*104),234,96,6,0x5841);
    }
    }
    sprite.unloadFont();
    lcd_PushColors(0, 0, 536, 240, (uint16_t*)sprite.getPointer());

   
  }

void setup() {
    pinMode(up, INPUT_PULLUP);
    pinMode(down, INPUT_PULLUP);
    for (int i = 0; i < 5; i++){
      pinMode(buttons[i], INPUT_PULLUP);
    } 

    Serial.begin(9600);
      rm67162_init();  // amoled lcd initialization
      lcd_setRotation(1);
      sprite.createSprite(536,240);
      sprite.setSwapBytes(1);
     
      sprite.loadFont(NotoSansBold15);
      lcd_brightness(150);

        for (int i = 0; i < 52; i++) 
        deck[i] = i;
    
    randomSeed(analogRead(0));
}

void readButtons()
{
 for(int i=0;i<5;i++){
   if(digitalRead(buttons[i]) == 0){
       if (debB[i] == 0) {
               debB[i] = 1;
               held[i]=!held[i];}
   }else debB[i]=0;
  }


 if (digitalRead(up) == 0 && digitalRead(down) == 1) {
             if (deb == 0) {
               deb = 1;
               if(fase==5){
               bet=bet*2; if(bet>score) bet=2;}
            }
        } else deb = 0;

        if (digitalRead(up) == 1 && digitalRead(down) == 0) {
            if (deb2 == 0) {
              deb2 = 1;
              if(fase==2) fase=3; else fase=0;
            }
        } else deb2 = 0;

     

}

void loop() {

  if(fase==0)
      {
      if(started==0)
            {
            timePassed=millis();
            started=1;
              for(int i=0; i<5;i++)
              hand[i]=52;
        }

       if(millis()>timePassed+period)
       {started=0; fase=1;} 
        
      }
   
    if(fase==1)
      {
        deal++;
        score=score-bet;
        win="deal";
        shuffle(deck, 52);
       // Deal 5 cards to the player
        for (int i = 0; i < 5; i++) {
        hand[i] = deck[i];
        held[i]=0;}
        fase=2;
      }

      if(fase==2)
      {
        readButtons();
      }

      if(fase==3)
      {
      if(started2==0)
            {
            timePassed2=millis();
            started2=1;
              for(int i=0; i<5;i++)
              if (!held[i])
              hand[i]=52;
        }

       if(millis()>timePassed2+period)
       {started2=0; fase=4;} 
        
      }
    
    if(fase==4){
    // Replace cards that are not held
    for (int i = 0; i < 5; i++) {
        if (!held[i]) {
            hand[i] = deck[i + 5];
        }
    }

    for(int i=0;i<5;i++)
    tmpHand[i]=hand[i];

    handRank = evaluateHand(hand);
    if(handRank>1) score=score+(bet*multi[handRank-2]);
    Serial.print("Hand Rank: ");
    switch (handRank) {
        case 1:
            win="High Card";
            break;
        case 2:
            win="One Pair";
            break;
        case 3:
            win="Two Pairs";
            break;
        case 4:
            win="Three of a Kind";
            break;
        case 5:
            win="Straight";
            break;
        case 6:
            win="Flush";
            break;
        case 7:
            win="Full House";
            break;
        case 8:
            win="Four of a Kind";
            break;
        default:
            win="nothing";
    }
    fase=5;
    
    for(int i=0;i<5;i++)
    hand[i]=tmpHand[i];
    }

   

    if(fase==5)
    readButtons();

     draw();
    
}
