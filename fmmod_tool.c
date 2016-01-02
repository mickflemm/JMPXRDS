/*
 * JMPXRDS, an FM MPX signal generator with RDS support on
 * top of Jack Audio Connection Kit - FMMOD runtime configuration tool
 *
 * Copyright (C) 2015 Nick Kossifidis <mickflemm@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>	/* For strtol */
#include <stdio.h>	/* For printf / snprintf */
#include <string.h>	/* For memset / strncmp */
#include <unistd.h>	/* For ftruncate */
#include <sys/mman.h>	/* For shm_open */
#include <fcntl.h>	/* For O_* constants */
#include <sys/stat.h>	/* For mode constants */
#include "fmmod.h"


#define TEMP_BUF_LEN	3 + 1

void
usage(char *name)
{
	printf("FMMOD Configuration tool for JMPXRDS\n");
	printf("\nUsage: %s -g or [<parameter> <value>] pairs\n", name);
	printf("\nParameters:\n"
		"\t-g\t\tGet current values\n"
		"\t-a   <int>\tSet audio gain precentage (default is 40%%)\n"
		"\t-m   <int>\tSet MPX gain percentage (default is 100%%)\n"
		"\t-p   <int>\tSet pilot gain percentage (default is 8%%)\n"
		"\t-r   <int>\tSet RDS gain percentage (default is 2%%)\n"
		"\t-s   <int>\tSet stereo mode 0 -> DSBSC (default), 1-> SSB (Hartley), 2-> SSB (Weaver)\n");
}

/* Yes it's ugly... */
int
main(int argc, char *argv[])
{
	int ctl_fd = 0;
	int ret = 0;
	int i = 0;
	char temp[TEMP_BUF_LEN] = {0};
	struct fmmod_control *ctl = NULL;

	if(argc < 2)
		usage(argv[0]);

	ctl_fd = shm_open(FMMOD_CTL_SHM_NAME, O_RDWR, 0600);
	if(ctl_fd < 0) {
		ret = FMMOD_ERR_SHM_ERR;
		goto cleanup;
	}

	ret = ftruncate(ctl_fd, sizeof(struct fmmod_control));
	if(ret != 0) {
		ret = FMMOD_ERR_SHM_ERR;
		goto cleanup;
	}


	ctl = (struct fmmod_control*)
				mmap(0, sizeof(struct fmmod_control),
				     PROT_READ | PROT_WRITE, MAP_SHARED,
				     ctl_fd, 0);
	if(ctl == MAP_FAILED) {
		ret = FMMOD_ERR_SHM_ERR;
		goto cleanup;
	}

	for(i = 1; i < argc; i++) {
		if(!strncmp(argv[i], "-g", 3)) {
			printf("Current config:\n"
			"\tAudio:   %i%%\n"
			"\tMPX:     %i%%\n"
			"\tPilot:   %i%%\n"
			"\tRDS:     %i%%\n"
			"\tStereo:  %s\n"
			"Current gains:"
			"\tAudio Left:  %f\n"
			"\tAudio Right: %f\n"
			"\tMPX:         %f\n",
			(int) (100 * ctl->audio_gain),
			(int) (100 * ctl->mpx_gain),
			(int) (100 * ctl->pilot_gain),
			(int) (100 * ctl->rds_gain),
			ctl->stereo_modulation == FMMOD_DSB ? "DSBSC" :
			ctl->stereo_modulation == FMMOD_SSB_HARTLEY ? "SSB (Hartley)" :
			"SSB (Weaver)",
			ctl->avg_audio_in_l,
			ctl->avg_audio_in_r,
			ctl->avg_mpx_out);
		}
		if(!strncmp(argv[i], "-a", 3)) {
			if(i < argc - 1) {
				memset(temp, 0, TEMP_BUF_LEN);
				snprintf(temp, 4, "%s", argv[++i]);
				ctl->audio_gain = (float) (strtol(temp, NULL, 10)) / 100.0;
				printf("New audio gain:  \t%i%%\n",(int) (100 * ctl->audio_gain));
			} else {
				usage(argv[0]);
				goto cleanup;
			}
		}
		if(!strncmp(argv[i], "-m", 3)) {
			if(i < argc - 1) {
				memset(temp, 0, TEMP_BUF_LEN);
				snprintf(temp, 4, "%s", argv[++i]);
				ctl->mpx_gain = (float) (strtol(temp, NULL, 10)) / 100.0;
				printf("New MPX gain:  \t%i%%\n",(int) (100 * ctl->mpx_gain));
			} else {
				usage(argv[0]);
				goto cleanup;
			}
		}
		if(!strncmp(argv[i], "-p", 3)) {
			if(i < argc - 1) {
				memset(temp, 0, TEMP_BUF_LEN);
				snprintf(temp, 4, "%s", argv[++i]);
				ctl->pilot_gain = (float) (strtol(temp, NULL, 10)) / 100.0;
				printf("New pilot gain:  \t%i%%\n",(int) (100 * ctl->pilot_gain));
			} else {
				usage(argv[0]);
				goto cleanup;
			}
		}
		if(!strncmp(argv[i], "-r", 3)) {
			if(i < argc - 1) {
				memset(temp, 0, TEMP_BUF_LEN);
				snprintf(temp, 4, "%s", argv[++i]);
				ctl->rds_gain = (float) (strtol(temp, NULL, 10)) / 100.0;
				printf("New RDS gain:  \t%i%%\n",(int) (100 * ctl->rds_gain));
			} else {
				usage(argv[0]);
				goto cleanup;
			}
		}
		if(!strncmp(argv[i], "-s", 3)) {
			if(i < argc - 1) {
				memset(temp, 0, TEMP_BUF_LEN);
				snprintf(temp, 4, "%s", argv[++i]);
				ctl->stereo_modulation = strtol(temp, NULL, 10) & 0x3;
				printf("Set stereo modulation:  \t%i\n", ctl->stereo_modulation);
			} else {
				usage(argv[0]);
				goto cleanup;
			}
		}
	}

cleanup:
	if(ctl_fd > 0)
		close(ctl_fd);
	if(ctl)
		munmap(ctl, sizeof(struct fmmod_control));
	return ret;
}
