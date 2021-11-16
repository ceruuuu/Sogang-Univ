#ifndef _ALL
#define _ALL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct CUSTOMER{
    int number;
    char id[20];
    char pw[20];
    char name[20];
    char address[200];
    int balance;//잔액
    int state;
}customer;

typedef struct PRODUCT{
    int number;
    char p_name[20];
    char category[20];
    int price;
    int state;

}product;

typedef struct HISTORY{
    int order_n;
    int product_n;
    int customer_n;
    int state;
}history;

int CountReadfile(FILE* fpr);
void Customer_Readfile(FILE *fpr, customer *a, int n);
void Product_Readfile(FILE *fpr, product *b, int n);
void History_Readfile(FILE *fpr, history *c, int n);
void Customer_Signup();
void customer_inf_print();
int customer_interface(char*);
int Customer_login(customer *a_1);
int Manager_login();
int customer_interface(char *name);
int manager_interface();
int main_interface();
void print_customer_inf(customer a_1);
int pw_check(customer a_1);
customer m_customer_update(customer *a, int customer_number);
customer customer_update(customer a_1, int choose_n3);  
void rewrite_history(history *c, history c_1);
void rewrite_customer(customer *a, customer a_1);
void pro_new(product *b);
void pro_dlt(product *b);
void pro_rew(product *b);
void Banking(customer *a_1, customer *a);
int Product_Search(int loginFlag, product* b_1);
void Buying(customer a_1, int p_number, history *c);
void show_del(customer *a, customer a_1, product *b, history *c);
void cus_buylist(customer a_1, product *b, history *c);
void del(history* c);
int cnt(history* c, int hn, int n);
void show_sell(product *b, history *c);
void show_product(product *b, history *c);

#endif

