#include <application.h>

#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(D6,A2);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL};

typedef enum { role_ping_out = 1, role_pong_back=2 } role_e;
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

role_e role = role_pong_back;


void setup() {
	Serial.begin(57600); // make sure serial terminal is closed before booting the Core
	while(!Serial.available()); // wait for user to open serial terminal and press enter
	SERIAL("ROLE: RECEIVING\n\r");

	radio.begin();

	// Optionally, uncomment to increase the delay between retries & # of retries.
	// Delay is in 250us increments (4ms max), retries is 15 max.
	// radio.setRetries(15,15);

	SERIAL("\n\rRadio Setup\n\r");

	radio.setDataRate(RF24_1MBPS);
	radio.setCRCLength(RF24_CRC_8);
	radio.setPayloadSize(8);
	radio.setChannel(101);
	radio.setAutoAck(true);
	radio.openWritingPipe(pipes[0]);
	radio.openReadingPipe(1,pipes[1]);

	radio.startListening();
	radio.printDetails();
}

void loop() {
	if (role == role_ping_out) {
		radio.stopListening();

		// Re-open the pipes for Tx'ing
		radio.openWritingPipe(0x00F0F0F0F0);
		radio.openReadingPipe(1, pipes[1]);

		// Take the time, and send it.	This will block until complete
		unsigned long time = millis();
		SERIAL("Now sending %lu...", time);
		bool ok = radio.write(&time, sizeof(unsigned long));

		if (ok) {
			SERIAL("ok...\n\r");
		} else {
			SERIAL(" failed.\n\r");
		}

		delay(50);
		radio.startListening();
	}

	else if (role == role_pong_back) {
		radio.openWritingPipe(0x00F0F0F0F0);
		radio.openReadingPipe(1,0xF0F0F0F0D2LL);
		radio.startListening();
		/*if (radio.available()) {
			unsigned long got_time;
			bool done = false;
			while (!done) {
				done = radio.read( &got_time, sizeof(unsigned long) );
				Serial.print("Got payload ");
				Serial.println(got_time);
				delay(20);
			}
		}*/
		char got_payload[8];
		bool done=false;
		if (radio.available()) {
			while(!done){
				done = radio.read(&got_payload, sizeof(got_payload));
				Serial.print("Got payload: ");
				Serial.println(got_payload);
				delay(20);
			}
		}
	} else {
		SERIAL("Role not defined \n\r");
	}
}
