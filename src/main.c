#include <ncurses.h>
#include <string.h>
#include <pthread.h>
#include <SFML/Audio.h>

char* get_fancy_duration(int duration) {
	int hours = duration / 3600;
	int minutes = (duration % 3600) / 60;
	int seconds = duration % 60;

	static char fancy_duration_buffer[64];
	snprintf(fancy_duration_buffer, sizeof(fancy_duration_buffer), "%s%d:%s%d:%s%d", ((hours < 10) ? "0" : ""), hours, ((minutes < 10) ? "0" : ""), minutes, ((seconds < 10) ? "0" : ""), seconds);
	return fancy_duration_buffer;
}

void* thread_checkkey(void* data) {
	while(sfMusic_getStatus(data) != sfStopped) {
		int key = getch();
		if(key != ERR) {
			if(key == 'p') {
				if(sfMusic_getStatus(data) == sfPlaying) {
					sfMusic_pause(data);
				} else if(sfMusic_getStatus(data) == sfPaused) {
					sfMusic_play(data);
				}
			}
		}
	}
	return NULL;
}

int main(int argc, char** argv) {
	if(argc < 2) {
		fprintf(stderr, "[musayer] music file is not provided\n");
		fprintf(stderr, "[musayer usage] %s <filename>\n", argv[0]);
		return 1;
	}

	sfMusic* music = sfMusic_createFromFile(argv[1]);
	if(music == NULL) {
		return 1;
	}
	
	initscr();
	cbreak();
	noecho();

	printw("[musayer] Channels: %u\n", sfMusic_getChannelCount(music));
	refresh();
	printw("[musayer] Sample rate: %u\n", sfMusic_getSampleRate(music));
	refresh();
	printw("[musayer] Duration: %s\n", get_fancy_duration(sfTime_asSeconds(sfMusic_getDuration(music))));
	refresh();
	printw("[musayer] Press key P to pause/unpause.\n");
	refresh();

	sfMusic_play(music);

	pthread_t th_checkkey_id;
	pthread_create(&th_checkkey_id, NULL, thread_checkkey, music);
	
	while(sfMusic_getStatus(music) != sfStopped) {
		if(sfMusic_getStatus(music) == sfPlaying) {
			printw("%s\r", get_fancy_duration(sfTime_asSeconds(sfMusic_getPlayingOffset(music))));
			refresh();
		}
	}

	pthread_join(th_checkkey_id, NULL);
	endwin();

	sfMusic_destroy(music);
	return 0;
}