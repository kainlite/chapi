/*
 * =====================================================================================
 *
 *       Filename:  encode.h
 *
 *    Description:  String encoding library for twilio web posts/gets
 *
 *        Version:  1.0
 *        Created:  07/29/2012 07:08:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  William Dignazio (slackwill), slackwill@csh.rit.edu
 *     Repository:  https://github.com/WillDignazio/twilio-c
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef ENCODE_GAURD_H
#define ENCODE_GAURD_H 

int html_encoded_size(char*); 
void html_encode(char*, char**); 

#endif 

