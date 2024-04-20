#line 1 "/home/ubuntu/proj/esp32_voc/esp32_voc/src/lg/lg.cpp"
#include <Arduino.h>
#include "lg.h"

const char version [] = "PcRead 201114a";

int debug = 0;

void lg(String msg) {
   Serial.println(msg);
}

// ---------------------------------------------------------
// toggle output bit
int readString (char *s,int   maxChar ){
    int  n = 0;

    Serial.print ("> ");
    do {
        if (Serial.available()) {
            int c    = Serial.read ();

            if ('\n' == c)
                break;

            s [n++] = c;
            if (maxChar == n)
                break;
        }
    } while (true);

    return n;
}

// -----------------------------------------------------------------------------
// process single character commands from the PC

char s [MAX_CHAR] = {};

int  analogPin = 0;

void pcRead (void){

    static int  val = 0;

    if (Serial.available()) {
        int c = Serial.read ();

        switch (c)  {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            val = c - '0' + (10 * val);
            break;

        case 'A':
            analogPin = val;
            Serial.print   ("analogPin = ");
            Serial.println (val);
            val = 0;
            break;

        case 'D':
            debug ^= 1;
            break;

        case 'I':
            pinMode (val, INPUT);
            Serial.print   ("pinMode ");
            Serial.print   (val);
            Serial.println (" INPUT");
            val = 0;
            break;

        case 'O':
            pinMode (val, OUTPUT);
            Serial.print   ("pinMode ");
            Serial.print   (val);
            Serial.println (" OUTPUT");
            val = 0;
            break;

        case 'P':
            pinMode (val, INPUT_PULLUP);
            Serial.print   ("pinMode ");
            Serial.print   (val);
            Serial.println (" INPUT_PULLUP");
            val = 0;
            break;


        case 'a':
            Serial.print   ("analogRead: ");
            Serial.println (analogRead (val));
            val = 0;
            break;

        case 'c':
            digitalWrite (val, LOW);
            Serial.print   ("digitalWrite: LOW  ");
            Serial.println (val);
            val = 0;
            break;

        case 'p':
#if !defined(ARDUINO_ARCH_ESP32)
            analogWrite (analogPin, val);
            Serial.print   ("analogWrite: pin ");
            Serial.print   (analogPin);
            Serial.print   (", ");
            Serial.println (val);
            val = 0;
#endif
            break;

        case 'r':
            Serial.print   ("digitalRead: pin ");
            Serial.print   (val);
            Serial.print   (", ");
            Serial.println (digitalRead (val));
            val = 0;
            break;

        case 's':
            digitalWrite (val, HIGH);
            Serial.print   ("digitalWrite: HIGH ");
            Serial.println (val);
            val = 0;
            break;

        case 't':
            Serial.print   ("pinToggle ");
            Serial.println (val);
            digitalWrite (val, ! digitalRead (val));
            val = 0;
            break;

        case 'v':
            Serial.print ("\nversion: ");
            Serial.println (version);
            break;

        case '\n':          // ignore
            break;

        case '"':
            while ('\n' != Serial.read ())     // discard linefeed
                ;

            readString (s, MAX_CHAR-1);
            Serial.println (s);
            break;

        case '?':
            Serial.println ("\npcRead:\n");
            Serial.println ("    [0-9] append to #");
            Serial.println ("    A # - set analog pin #");
            Serial.println ("    D # - set debug to #");
            Serial.println ("    I # - set pin # to INPUT");
            Serial.println ("    O # - set pin # to OUTPUT");
            Serial.println ("    P # - set pin # to INPUT_PULLUP");
            Serial.println ("    a # - analogRead (pin #)");
            Serial.println ("    c # - digitalWrite (pin #, LOW)");
            Serial.println ("    p # -- analogWrite (analogPin, #)");
            Serial.println ("    r # - digitalRead (pin #)");
            Serial.println ("    s   - digitalWrite (pin #, HIGH)");
            Serial.println ("    t   -- toggle pin # output");
            Serial.println ("    v   - print version");
            Serial.println ("    \"   - read string");
            Serial.println ("    ?   - list of commands");
            break;

        default:
            Serial.print ("unknown char ");
            Serial.println (c,HEX);
            break;
        }
    }
}

/*
// -----------------------------------------------------------------------------
void loop (void)
{
    pcRead ();
}

// -----------------------------------------------------------------------------
void
setup (void)
{
    Serial.begin(115200);

    Serial.println (version);
#if defined(ARDUINO_ARCH_ESP32)
    Serial.println ("esp32");
#endif
}
*/