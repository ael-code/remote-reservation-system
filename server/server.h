//struct to store server option
struct server_option{
	int port;  								// 0 means random port
	int backlog;  							// 50 default
	char colored;							// 0 = false; 1=true; dafault 0
	char verbose;							// 0 = false; 1=true; default 0
	char * file;							
	unsigned int pwd_length;
	unsigned int map_rows;
	unsigned int map_cols;
};

