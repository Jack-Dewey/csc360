#ifndef CUSTOMER_INFO_H
#define CUSTOMER_INFO_H

typedef struct customer_info customer_info;

struct customer_info{ /// use this struct to record the customer information read from customers.txt
    int user_id;
	int class_type;
	int service_time;
	int arrival_time;
	double start;
	double end;
	double service_end;
};

#endif