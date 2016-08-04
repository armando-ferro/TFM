/*
 * Types.h
 *
 *  Created on: 19/12/2015
 *      Author: ainhoa
 */

#ifndef TYPES_H_
#define TYPES_H_

/*tipos de mensaje de la capa de enlace*/
const short e_msg_t = 0;
const short e_ack_t = 1;
const short e_nack_t = 2;

/*tipos de mensaje de la capa de red*/
const short r_msg_t = 0;
const short r_ack_t = 1;
const short r_nack_t = 2;

/*tipos de mensjae de la capa de transporte*/
const short t_msg_t = 0;
const short t_ack_t = 1;
const short t_nack_t = 2;

/*tipos de mensaje de la capa de aplicación*/
const short a_msg_t = 0;
const short a_ack_t = 1;
const short a_nack_t = 2;


#endif /* TYPES_H_ */
