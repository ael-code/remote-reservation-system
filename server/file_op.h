#ifndef __FILE_OP__
#define __FILE_OP__

void save_server_opt();

void load_server_opt();

int file_exist(char * file);

void file_close();

int save_reservation_array(unsigned int arr_dim, struct res_entry * arr,unsigned int chiav_dim);

int load_reservation_array(unsigned int arr_dim, struct res_entry * arr,unsigned int chiav_dim);
#endif
