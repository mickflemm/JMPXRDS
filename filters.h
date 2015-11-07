/*
 * JMPXRDS, an FM MPX signal generator with RDS support on
 * top of Jack Audio Connection Kit - Various filter implementations
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

/* A generic sinc FIR Low pass filter, multiplied by
 * a Blackman - Harris window */
#define FIR_FILTER_SIZE		513
#define	FIR_FILTER_HALF_SIZE	(FIR_FILTER_SIZE - 1) / 2

struct fir_filter_data {
	float fir_coeffs[FIR_FILTER_HALF_SIZE];
	float fir_buff_l[FIR_FILTER_SIZE];
	float fir_buff_r[FIR_FILTER_SIZE];
	int fir_index;
};

int fir_filter_init(struct fir_filter_data *, int cutoff_freq, int sample_rate);
float fir_filter_apply(struct fir_filter_data *, float sample, int chan_idx);
void fir_filter_update(struct fir_filter_data *);

/* FM Preemphasis IIR filter */
struct fmpreemph_filter_data {
	float iir_inbuff_l[2];
	float iir_outbuff_l[2];
	float iir_inbuff_r[2];
	float iir_outbuff_r[2];
	float iir_ataps[2];
	float iir_btaps[2];
};

/* -Only used as part of the combined audio filter- */

/* Combined audio filter */
struct audio_filter {
	struct fir_filter_data audio_lpf;
	struct fmpreemph_filter_data fm_preemph;
};

void audio_filter_init(struct audio_filter *, int cutoff_freq, int sample_rate,
							int preemph_tau_usecs);
void audio_filter_update(struct audio_filter *);

float audio_filter_apply(struct audio_filter *, float sample, int chan_idx);

/* IIR filter for the Weaver modulator (SSB) */
#define SSB_FILTER_TAPS 10
#define	SSB_FILTER_SIZE SSB_FILTER_TAPS + 1
#define SSB_FILTER_GAIN 5.279294303e+02
#define SSB_FILTER_REVERSE_GAIN (1 / SSB_FILTER_GAIN)

struct ssb_filter_data {
	float iir_inbuff_l[SSB_FILTER_SIZE];
	float iir_outbuff_l[SSB_FILTER_SIZE];
	float iir_inbuff_r[SSB_FILTER_SIZE];
	float iir_outbuff_r[SSB_FILTER_SIZE];
	/* ataps are symmetric, store the bottom
	 * half part */
	float iir_ataps[SSB_FILTER_TAPS / 2 + 1];
	float iir_btaps[SSB_FILTER_TAPS];
};

int iir_ssb_filter_init(struct ssb_filter_data *iir);
float iir_ssb_filter_apply(struct ssb_filter_data *iir, float sample,
							int chan_idx);

/* Hilbert transformer for the Hartley modulator (SSB) */
#define HT_FIR_FILTER_SIZE		65
#define HT_FIR_FILTER_GAIN 1.568367973e+00
#define HT_FIR_FILTER_TAPS		(HT_FIR_FILTER_SIZE - 1)
#define HT_FIR_FILTER_REVERSE_GAIN (1 / HT_FIR_FILTER_GAIN)

struct hilbert_transformer_data {
	float fir_coeffs[HT_FIR_FILTER_SIZE];
	float fir_buff[HT_FIR_FILTER_SIZE];
};

int hilbert_transformer_init(struct hilbert_transformer_data *ht);
float hilbert_transformer_apply(struct hilbert_transformer_data *ht, float sample);