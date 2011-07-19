/* @@--
 * 
 * Copyright (C) 2011 Alberto Vigata
 *       
 * This file is part of vgtmpeg
 * 
 * a Versed Generalist Transcoder
 * 
 * vgtmpeg is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * vgtmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __NLDUMP_FORMAT_H
#define __NLDUMP_FORMAT_H

static int get_bit_rate(AVCodecContext *ctx)
{
    int bit_rate;
    int bits_per_sample;

    switch(ctx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
    case AVMEDIA_TYPE_DATA:
    case AVMEDIA_TYPE_SUBTITLE:
    case AVMEDIA_TYPE_ATTACHMENT:
        bit_rate = ctx->bit_rate;
        break;
    case AVMEDIA_TYPE_AUDIO:
        bits_per_sample = av_get_bits_per_sample(ctx->codec_id);
        bit_rate = bits_per_sample ? ctx->sample_rate * ctx->channels * bits_per_sample : ctx->bit_rate;
        break;
    default:
        bit_rate = 0;
        break;
    }
    return bit_rate;
}

static void avcodec_nlstring(char *buf, int buf_size, AVCodecContext *enc, int encode) {
    const char *codec_name;
    AVCodec *p;
    char buf1[32];
    int bitrate;
    AVRational display_aspect_ratio;

    if (encode)
        p = avcodec_find_encoder(enc->codec_id);
    else
        p = avcodec_find_decoder(enc->codec_id);

    if (p) {
        codec_name = p->name;
    } else if (enc->codec_id == CODEC_ID_MPEG2TS) {
        /* fake mpeg2 transport stream codec (currently not
           registered) */
        codec_name = "mpeg2ts";
    } else if (enc->codec_name[0] != '\0') {
        codec_name = enc->codec_name;
    } else {
        /* output avi tags */
        if(   isprint(enc->codec_tag&0xFF) && isprint((enc->codec_tag>>8)&0xFF)
           && isprint((enc->codec_tag>>16)&0xFF) && isprint((enc->codec_tag>>24)&0xFF)){
            snprintf(buf1, sizeof(buf1), "%c%c%c%c / 0x%04X",
                     enc->codec_tag & 0xff,
                     (enc->codec_tag >> 8) & 0xff,
                     (enc->codec_tag >> 16) & 0xff,
                     (enc->codec_tag >> 24) & 0xff,
                      enc->codec_tag);
        } else {
            snprintf(buf1, sizeof(buf1), "0x%04x", enc->codec_tag);
        }
        codec_name = buf1;
    }

    switch(enc->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        FFMSG_LOG( FFMSG_STRING_FMT(codectype), "video" );
        FFMSG_LOG( FFMSG_STRING_FMT(codecname), codec_name );

        if (enc->pix_fmt != PIX_FMT_NONE) {
            FFMSG_LOG( FFMSG_STRING_FMT(picfmt), avcodec_get_pix_fmt_name(enc->pix_fmt) );
        }

        if (enc->width) {
            FFMSG_LOG( FFMSG_INT32_FMT(width), enc->width );
            FFMSG_LOG( FFMSG_INT32_FMT(height),enc->height );

            if (enc->sample_aspect_ratio.num) {
                av_reduce(&display_aspect_ratio.num, &display_aspect_ratio.den,
                          enc->width*enc->sample_aspect_ratio.num,
                          enc->height*enc->sample_aspect_ratio.den,
                          1024*1024);
                FFMSG_LOG( FFMSG_INT32_FMT(darnum), display_aspect_ratio.num );
                FFMSG_LOG( FFMSG_INT32_FMT(darden), display_aspect_ratio.den );
                FFMSG_LOG( FFMSG_INT32_FMT(sarnum), enc->sample_aspect_ratio.num );
                FFMSG_LOG( FFMSG_INT32_FMT(sarden), enc->sample_aspect_ratio.den );
            }

            /* if(av_log_get_level() >= AV_LOG_DEBUG){ */
                /* int g= av_gcd(enc->time_base.num, enc->time_base.den); */
                /* snprintf(buf + strlen(buf), buf_size - strlen(buf), */
                     /* ", %d/%d", */
                     /* enc->time_base.num/g, enc->time_base.den/g); */
            /* } */
        }
        break;
    case AVMEDIA_TYPE_AUDIO:
        FFMSG_LOG( FFMSG_STRING_FMT(codectype), "audio" );
        FFMSG_LOG( FFMSG_STRING_FMT(codecname), codec_name );
        if (enc->sample_rate) {
                FFMSG_LOG( FFMSG_INT32_FMT(samplerate), enc->sample_rate  );
        }
        //avcodec_get_channel_layout_string(buf , buf_size , enc->channels, enc->channel_layout);
        FFMSG_LOG( FFMSG_INTEGER_FMT(channel_layout),  enc->channel_layout );

        if (enc->sample_fmt != SAMPLE_FMT_NONE) {
            FFMSG_LOG( FFMSG_STRING_FMT(audfmt), avcodec_get_sample_fmt_name(enc->sample_fmt));
        }
        break;
    case AVMEDIA_TYPE_DATA:
        FFMSG_LOG( FFMSG_STRING_FMT(codectype), "data" );
        FFMSG_LOG( FFMSG_STRING_FMT(codecname), codec_name );
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        FFMSG_LOG( FFMSG_STRING_FMT(codectype), "subtitle" );
        FFMSG_LOG( FFMSG_STRING_FMT(codecname), codec_name );
        break;
    case AVMEDIA_TYPE_ATTACHMENT:
        FFMSG_LOG( FFMSG_STRING_FMT(codectype), "attachment" );
        FFMSG_LOG( FFMSG_STRING_FMT(codecname), codec_name );
        break;
    default:
        FFMSG_LOG( FFMSG_STRING_FMT(codectype), "invalid" );
        FFMSG_LOG( FFMSG_INT32_FMT(codecname), enc->codec_type );
        return;
    }

    bitrate = get_bit_rate(enc);
    if (bitrate != 0) {
        FFMSG_LOG( FFMSG_INT32_FMT(bitrate), bitrate );
    }
}



static void dump_stream_nlformat(AVFormatContext *ic, int i, int index, int is_output)
{
    FFMSG_LOG( FFMSG_NODE_START_FMT("stream_%d_%d"), index, i );
    FFMSG_LOG( FFMSG_INT32_FMT(index), index );
    FFMSG_LOG( FFMSG_INT32_FMT(stid), i );

    char buf[256];
    int flags = (is_output ? ic->oformat->flags : ic->iformat->flags);
    AVStream *st = ic->streams[i];
    int g = av_gcd(st->time_base.num, st->time_base.den);
    AVMetadataTag *lang = av_metadata_get(st->metadata, "language", NULL, 0);
    avcodec_nlstring(buf, sizeof(buf), st->codec, is_output);
 
 
    if (lang) {
        FFMSG_LOG( FFMSG_STRING_FMT(lang), lang->value );
    }
 
    /* av_log(NULL, AV_LOG_DEBUG, ", %d, %d/%d", st->codec_info_nb_frames, st->time_base.num/g, st->time_base.den/g); */
    /* av_log(NULL, AV_LOG_INFO, ": %s", buf); */
    /* if (st->sample_aspect_ratio.num && // default */
        /* av_cmp_q(st->sample_aspect_ratio, st->codec->sample_aspect_ratio)) { */
        /* AVRational display_aspect_ratio; */
        /* av_reduce(&display_aspect_ratio.num, &display_aspect_ratio.den, */
                  /* st->codec->width*st->sample_aspect_ratio.num, */
                  /* st->codec->height*st->sample_aspect_ratio.den, */
                  /* 1024*1024); */
        /* av_log(NULL, AV_LOG_INFO, ", PAR %d:%d DAR %d:%d", */
                 /* st->sample_aspect_ratio.num, st->sample_aspect_ratio.den, */
                 /* display_aspect_ratio.num, display_aspect_ratio.den); */
    /* } */
    if(st->codec->codec_type == AVMEDIA_TYPE_VIDEO){
        if(st->avg_frame_rate.den && st->avg_frame_rate.num) {
            FFMSG_LOG( FFMSG_INT32_FMT(avg_framerate_num), st->avg_frame_rate.num );
            FFMSG_LOG( FFMSG_INT32_FMT(avg_framerate_den), st->avg_frame_rate.den );
        }

        if(st->r_frame_rate.den && st->r_frame_rate.num) {
            FFMSG_LOG( FFMSG_INT32_FMT(r_framerate_num), st->r_frame_rate.num );
            FFMSG_LOG( FFMSG_INT32_FMT(r_framerate_den), st->r_frame_rate.den );
        }
        if(st->time_base.den && st->time_base.num) {
            FFMSG_LOG( FFMSG_INT32_FMT(mux_timebase_num), st->time_base.num );
            FFMSG_LOG( FFMSG_INT32_FMT(mux_timebase_den), st->time_base.den );
        }

        if(st->codec->time_base.den && st->codec->time_base.num) {
            FFMSG_LOG( FFMSG_INT32_FMT(codec_timebase_num), st->codec->time_base.num );
            FFMSG_LOG( FFMSG_INT32_FMT(codec_timebase_den), st->codec->time_base.den );
        }
    }
    FFMSG_LOG( FFMSG_INT32_FMT(inspected_frame_count), st->codec_info_nb_frames );

    FFMSG_LOG( FFMSG_NODE_STOP_FMT("stream_%d_%d"), index, i );
}


static void dump_nlformat(AVFormatContext *ic,
                 int index,
                 const char *url,
                 int is_output)
{
    int i,rscount;
    uint8_t *printed = av_mallocz(ic->nb_streams);
    if (ic->nb_streams && !printed)
        return;

    /* stream info */
    av_log(NULL, AV_LOG_INFO, FFMSG_START );
    FFMSG_LOG( FFMSG_INT32_FMT(version_major), FFMSG_VERSION_MAJOR );
    FFMSG_LOG( FFMSG_INT32_FMT(version_minor), FFMSG_VERSION_MINOR );
    FFMSG_LOG( FFMSG_STRING_FMT(msgtype), FFMSG_MSGTYPE_STREAMINFO );

    av_log(NULL, AV_LOG_INFO, FFMSG_NODE_START(muxinfo) );

    av_log(NULL, AV_LOG_INFO, FFMSG_STRING_FMT(direction), is_output ? "output" : "input" );
    av_log(NULL, AV_LOG_INFO, FFMSG_INTEGER_FMT(index), (int64_t)index );
    av_log(NULL, AV_LOG_INFO, FFMSG_INTEGER_FMT(timebase), (int64_t)AV_TIME_BASE );
    av_log(NULL, AV_LOG_INFO, FFMSG_STRING_FMT(mux_format), is_output ? ic->oformat->name : ic->iformat->name  );

    av_log(NULL, AV_LOG_INFO, FFMSG_INTEGER_FMT(program_count), (int64_t)ic->nb_programs );
    FFMSG_LOG( FFMSG_INT32_FMT(stream_count), ic->nb_streams );

    if (!is_output) {
        av_log(NULL, AV_LOG_INFO, FFMSG_INTEGER_FMT(duration), ic->duration != AV_NOPTS_VALUE ? ic->duration : 0 );
        av_log(NULL, AV_LOG_INFO, FFMSG_INTEGER_FMT(start_time), ic->start_time != AV_NOPTS_VALUE ? ic->start_time : 0 );
        if( ic->bit_rate ) {
            av_log(NULL, AV_LOG_INFO, FFMSG_INTEGER_FMT(bitrate), (int64_t)ic->bit_rate );
        }        

    }

    if(ic->nb_programs) {
        FFMSG_LOG( FFMSG_NODE_START(programs) );
        int j, k, total = 0;
        for(j=0; j<ic->nb_programs; j++) {
            AVMetadataTag *name = av_metadata_get(ic->programs[j]->metadata,  "name", NULL, 0);
            FFMSG_LOG( FFMSG_NODE_START_FMT("id%d"), ic->programs[j]->id );
            FFMSG_LOG( FFMSG_INTEGER_FMT(id), (int64_t) ic->programs[j]->id );
            FFMSG_LOG( FFMSG_STRING_FMT(name), name ? name->value : ""  );

            for(k=0; k<ic->programs[j]->nb_stream_indexes; k++) {
                dump_stream_nlformat(ic, ic->programs[j]->stream_index[k], index, is_output);
                printed[ic->programs[j]->stream_index[k]] = 1;
            }
            total += ic->programs[j]->nb_stream_indexes;

            FFMSG_LOG( FFMSG_NODE_STOP_FMT("id%d"), ic->programs[j]->id );
        }
        /* if (total < ic->nb_streams) */
            /* av_log(NULL, AV_LOG_INFO, "  No Program\n"); */
        FFMSG_LOG( FFMSG_NODE_STOP(programs) );
    }

    FFMSG_LOG( FFMSG_NODE_START(streams) );
    rscount = 0;
    for(i=0;i<ic->nb_streams;i++)
        if (!printed[i]) {
            dump_stream_nlformat(ic, i, index, is_output);
            rscount++;
        }

    FFMSG_LOG( FFMSG_NODE_STOP(streams) );
 
    FFMSG_LOG( FFMSG_INT32_FMT(rawstream_count), rscount );

    av_log(NULL, AV_LOG_INFO, FFMSG_NODE_STOP(muxinfo) );
    av_log(NULL, AV_LOG_INFO, FFMSG_STOP );
    av_free(printed); 
}


#endif
