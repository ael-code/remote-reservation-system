#ifndef __FILE_OP__
#define __FILE_OP__

#include "reservation.h"

int file_exist(char * file);

int file_close();

int file_open();

void save_server_opt();

void load_server_opt();

int save_reservation_array(unsigned int arr_dim, struct res_entry * arr,unsigned int chiav_dim);

int load_reservation_array(unsigned int arr_dim, struct res_entry * arr,unsigned int chiav_dim);

int save_delta_del(unsigned int index);

int save_delta_add(unsigned int index, struct res_entry * reservation);

int load_delta();

#endif
