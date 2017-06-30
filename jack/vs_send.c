// gcc -o jack_midi_dump -Wall midi_dump.c -ljack -pthread

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <inttypes.h>
#include <jack/jack.h>
#include <jack/midiport.h>
#include <jack/ringbuffer.h>

#ifndef MAX
#define MAX(a,b) ( (a) < (b) ? (b) : (a) )
#endif

static jack_port_t* port;
static jack_port_t* outport;
static jack_ringbuffer_t *rb = NULL;
static pthread_mutex_t msg_thread_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t data_ready = PTHREAD_COND_INITIALIZER;

static int keeprunning = 1;
static uint64_t monotonic_cnt = 0;

#define RBSIZE 512

typedef struct {
	uint8_t  buffer[128];
	uint32_t size;
	uint32_t tme_rel;
	uint64_t tme_mon;
} midimsg;


//Note: Midi CC value 64 = Speed value 0
const int vSpeedValue[] = { //Pitches (speed) for each note
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  19,  20,  21,  22,  23,  24,  25,  
26, 27, 28, 29, 30, 32, 34, 37, 40, 43, 45, 48, 51, 53, 56, 59, 61, 64, 67, 69, 72, 75, 78, 80, 83, 85, 88, 91, 93, 96, 97, 99, 100, 102, 103, 104, 106, 107, 108, 109, 111, 112
};

#define VOICES 4

int
process (jack_nframes_t frames, void* arg)
{
	void* buffer;
	void* outbuf;

	char data[3];
	char note_on [3] = {0x90, 0, 64};
	char cc_vel [3] = {0xb0, 43, 0};
	char pc [3] = {0xc0, 0};

	static char kp[VOICES];
	static char cur_note[VOICES];
	int max_voice;
	int min_voice;
	int sel_voice;
	int note;

	int v;

	jack_nframes_t N;
	jack_nframes_t i;

	buffer = jack_port_get_buffer (port, frames);
	assert (buffer);
	outbuf = jack_port_get_buffer (outport, frames);
	assert (outbuf);
 	jack_midi_clear_buffer(outbuf);

	N = jack_midi_get_event_count (buffer);
	for (i = 0; i < N; ++i) {
		jack_midi_event_t event;
		int r;

		r = jack_midi_event_get (&event, buffer, i);

		// translating pitch for volca sample 
		memcpy (data, event.buffer, MAX(event.size, 3));

		//printf("received frame %2x size %d\n", data[0], event.size);

		switch (data[0] &0xF0) {
		case 0x90 : // Note on
			note = data[1];

			sel_voice = 0;
			max_voice = 0;
			min_voice = kp[0];
			for (v = 0; v < VOICES; v++) {
				if (kp[v] < min_voice) {
					sel_voice = v;
					min_voice = kp[v];
				}
				if (kp[v] > max_voice) {
					max_voice = kp[v];
				}
			}		
			cur_note[sel_voice] = note;
			kp[sel_voice] = max_voice + 1;

			note_on [0] = 0x90 + sel_voice;
			cc_vel [0] = 0xb0 + sel_voice;

			cc_vel[2] = vSpeedValue[note];

			jack_midi_event_write (outbuf, 0, cc_vel, 3);
			jack_midi_event_write (outbuf, 1, note_on, 3);
			break;
		case 0x80 : // Note off
			note = data[1];
			for (v = 0; v < VOICES; v++) {
				if (cur_note[v] == note) {
					cur_note[v] = 0;
					kp[v] = 0;
				}
			}
			break;
		case 0xc0 : // Program change
			pc[1] = data[1];
			for (v=0; v < VOICES; v++) {
				pc[0] = 0xc0 + v;
				jack_midi_event_write (outbuf, 0, pc, 2);
			}	
			break;
		}

	}

	monotonic_cnt += frames;

	return 0;
}

static void wearedone(int sig) {
	keeprunning = 0;
}

int
main (int argc, char* argv[])
{
	jack_client_t* client;
	char const default_name[] = "vs_ctrl";
	char const * client_name;
	int time_format = 0;
	int r;

	int cn = 1;

	client_name = default_name;

	client = jack_client_open (client_name, JackNullOption, NULL);
	if (client == NULL) {
		fprintf (stderr, "Could not create JACK client.\n");
		exit (EXIT_FAILURE);
	}

	jack_set_process_callback (client, process, 0);

	port = jack_port_register (client, "input", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
	if (port == NULL) {
		fprintf (stderr, "Could not register port.\n");
		exit (EXIT_FAILURE);
	}

	outport = jack_port_register (client, "out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
	if (outport == NULL) {
		fprintf (stderr, "Could not register port.\n");
		exit (EXIT_FAILURE);
	}

	r = jack_activate (client);
	if (r != 0) {
		fprintf (stderr, "Could not activate client.\n");
		exit (EXIT_FAILURE);
	}


	uint64_t prev_event = 0;
	while (keeprunning) {

		sleep(1);

	}

	jack_deactivate (client);
	jack_client_close (client);
	jack_ringbuffer_free (rb);

	return 0;
}
