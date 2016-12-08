#include "mbed.h"
#include "m3pi.h"
#include <string>

using namespace std;

// Minimum and maximum motor speeds
#define MAX 0.25
#define MIN 0

// ROBOT ID || "Name"
#define R_ID 'p'
 
// PID terms
#define P_TERM 1
#define I_TERM 0
#define D_TERM 20

// MAX X and Y
int MAX_X = 5;
int MAX_Y = 5;

// Set up our m3pi
m3pi m3pi;

// Set up our pins
DigitalOut myled(LED1);
Serial xbee(p28, p27);
Serial pc(USBTX, USBRX);
DigitalOut rst(p26);
DigitalIn button(p21);

// Set up our x and y pos
int xpos = 0;
int ypos = 0;

// Define our methods
void atCommandMode();
bool canMove(int x, int y);
void runPath();

// Init our path
string path = "";
char RID;

LocalFileSystem local("local");

/*
TODO: get R_ID from a file

TODO: Have robots communicate with each other
while they move around the board

*/

void init() {
    m3pi.locate(0,1);
    m3pi.printf("Line PID");
    wait(2.0);
    m3pi.sensor_auto_calibrate(); 
}

// LEADER MAIN
int main() {
    
    button.mode(PullUp);
    
    rst = 0;
    wait(0.5);
    rst = 1;
    wait(0.5);
    
    atCommandMode(); // CONFIGURE SETTINGS USING PC
    init();
        
    while (1) {
        
        // while xbee is getting data
        while(xbee.readable()) {
            char c = xbee.getc();
            if(c == R_ID) {
                char d = xbee.getc();
                    while(d != 'f') {
                        // If they're numbers add them to a path
                        if(d >= '0' && d <= '9')
                            path += d;
                        // Continue to get chars until 'f'
                        d = xbee.getc();
                    }
            } else if(c == 'g') {
                runPath();
            } else {
                //default;   
            }
        }
    }
}

// Execute a path
void runPath() {
    // First two coords are the starting point
    m3pi.setCoords(path[0]-'0', path[1]-'0');
    // Run through the path
    for(int i=2; i<path.length(); i+=2) {
        m3pi.gotoPoint(path[i] - '0', path[i+1] - '0');
        wait(3.0);
    } 
    // Erase path and wait for a new one
    path = "";
}

bool canMove(int xd, int yd) {
    // Can't go off the grid
    if(xd > MAX_X || xd < 0 || yd > MAX_Y || yd < 0) return false;
    // Send a message to our follower
    xbee.putc('c'); // can I move...
    xbee.putc(xd); // .. to this x ..
    xbee.putc(yd); // .. and this y ..
    xbee.putc(xpos); // .. from this x ...
    xbee.putc(ypos); // ... and this y?
    
    m3pi.locate(0,0);
    m3pi.printf("%d, %d", xpos, ypos);
    
        char c = xbee.getc();
        switch(c) {
            case 'o': // okay to move
                return true;
            
            case 'n': // not okay to move
                return false;
        }
    return false;
}


// DO NOT EDIT:
// COMMAND MODE
void atCommandMode()
{
    pc.printf("start serial communication\n");
    // Allow at least 1 second to pass so xbee does not recieve or transmist any data to prevent problems if entering command mode
    wait(1.2);

    while (1) {
        // If terminal data available send through xbee
        if(pc.readable())
            xbee.putc(pc.getc());

        // If xbee gets data send to terminal
        while(xbee.readable())
            pc.printf("%c", xbee.getc());

        if (!button) { // exit the function
            wait(0.2);
            break;
        }
    }
}
