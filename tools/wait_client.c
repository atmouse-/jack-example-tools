/*
    Copyright (C) 2007 Paul Davis

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <errno.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#include <signal.h>
#include <stdlib.h>

#include <jack/jack.h>
#include <jack/metadata.h>
#include <jack/uuid.h>

#include <getopt.h>

jack_client_t *client;

char * my_name;
char client_name[300];

static void signal_handler(int sig)
{
	jack_client_close(client);
	fprintf(stderr, "signal received, exiting ...\n");
	exit(0);
}

static void
client_callback (const char* instance_client, int yn, void* arg)
{
	printf ("Client %s %s\n", instance_client, (yn ? "registered" : "unregistered"));
  if ((strcmp(instance_client, client_name) == 0) && (yn > 0)) {
    jack_client_close(client);
    fprintf(stderr, "client match, exiting ...\n");
    exit(0);
  }
}

void
show_usage(void)
{
	fprintf(stderr, "\nUsage: %s [options]\n", my_name);
	fprintf(stderr, "Check for jack existence, or wait client match\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "        -n, --name <name>     Client <name> to match\n");
	fprintf(stderr, "        -h, --help            Display this help message\n");
	fprintf(stderr, "For more information see http://jackaudio.org/\n");
}

int
main (int argc, char *argv[])
{
	jack_options_t options = JackNullOption;
	jack_status_t status;
	int c;
	int option_index;

	my_name = strrchr(argv[0], '/');
	if (my_name == 0) {
		my_name = argv[0];
	} else {
		my_name ++;
	}

	struct option long_options[] = {
		{ "name", 1, 0, 'n' },
		{ "help", 0, 0, 'h' },
		{ 0, 0, 0, 0 }
	};

  if (argc == 2) {
    snprintf( client_name, sizeof(client_name), "%s", argv[1] );
    goto begin;
  }
	if (argc < 2) {
		show_usage();
		return 1;
	}
	while ((c = getopt_long (argc, argv, "n:h", long_options, &option_index)) >= 0) {
		switch (c) {
		case 'n':
			strcpy (client_name, optarg);
			break;
		case 'h':
			show_usage();
			return 1;
		default:
			show_usage();
			return 1;
		}
	}

begin:
	if ((client = jack_client_open ("event-monitor", options, &status, NULL)) == 0) {
		fprintf (stderr, "jack_client_open() failed, "
			 "status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf (stderr, "Unable to connect to JACK server\n");
		}
		return 1;
	}

	if (jack_set_client_registration_callback (client, client_callback, NULL)) {
		fprintf (stderr, "cannot set client registration callback\n");
		return 1;
	}
	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		return 1;
	}

#ifndef WIN32
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGHUP, signal_handler);
#endif
	signal(SIGABRT, signal_handler);
	signal(SIGTERM, signal_handler);

#ifdef WIN32
	Sleep(INFINITE);
#else
	sleep (-1);
#endif
	exit (0);
}

