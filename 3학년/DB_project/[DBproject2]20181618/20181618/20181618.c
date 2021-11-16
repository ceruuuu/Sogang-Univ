#include <stdio.h>
#include "mysql.h"
#include <stdlib.h>

#pragma comment(lib, "libmysql.lib")

const char* host = "127.0.0.1";
const char* user = "root";
const char* pw = "gksmf7070";
const char* db = "jong";

int main(void) {

	MYSQL* connection = NULL;
	MYSQL conn;
	MYSQL_RES* sql_result;
	MYSQL_ROW sql_row;

	if (mysql_init(&conn) == NULL)
		printf("mysql_init() error!");

	connection = mysql_real_connect(&conn, host, user, pw, db, 3306, (const char*)NULL, 0);
	if (connection == NULL)
	{
		printf("%d ERROR : %s\n", mysql_errno(&conn), mysql_error(&conn));
		return 1;
	}

	else
	{
		printf("Connection Succeed\n");

		if (mysql_select_db(&conn, db))
		{
			printf("%d ERROR : %s\n", mysql_errno(&conn), mysql_error(&conn));
			return 1;
		}

		//===================================== create, insert

		FILE* f = fopen("20181618.txt", "r");
		char drop[20][100];
		int d_idx = 0;
		char inst[600];

		while (!feof(f)) {
			inst[0] = '\0';
			fgets(inst, 600, f);

			if (inst[0] == 'd') {
				strcpy(drop[d_idx++], inst);
				continue;
			}
			mysql_query(connection, inst);
		}

		fclose(f);
		//===========================================
		//===========================================


		const char* query;
		char qu[500];
		int state;
		int flag;
		int choice, num, year, month;
		char name[20];

		while (1) {
			printf("------- SELECT QUERY TYPES -------\n");
			printf("\t1. TYPE I\n");
			printf("\t2. TYPE II\n");
			printf("\t3. TYPE III\n");
			printf("\t4. TYPE IV\n");
			printf("\t5. TYPE V\n");
			printf("\t0. QUIT\n");

			printf("Which type of query? >");
			scanf("%d", &choice);
			if (choice == 0) break;

			switch (choice) {
			case 1:
				printf("---- TYPE I ----\n");
				while (1) {
					printf("Input the number of truck : ");		// 버스 번호 입력
					scanf("%d", &num); 
					if (num != 1721) {
						printf("Truck %d is not destroyed.\n\n", num);
						continue;
					}
					else if (num == 0) break;

					while (1) {

						printf("------- Subtypes in TYPE I -------\n");
						printf("\t1. TYPE I-1\n");
						printf("\t2. TYPE I-2\n");
						printf("\t3. TYPE I-3\n");

						printf("Which type of query? >");
						scanf("%d", &choice);

						if (choice == 0) { // 종료
							break;
						}
						else if (choice == 1) {
							printf("\n---- TYPE I-1 ----\n");
							printf("** Find all customers who had a package on the truck at the time of the crach. **\n");

							//쿼리 넣고 이름 출력

							query = "select c_name from customer, package, trace where customer.customer_ID = package.customer_ID and package.tracking_number = trace.tracking_number and t_date = '2020-03-07' and license_plate = '1721' and state = 'in transit'";
							flag = 0;
							state = 0;
							state = mysql_query(connection, query);
							if (state == 0)
							{
								printf("Customer name : ");

								sql_result = mysql_store_result(connection);
								while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
								{
					
									if (flag == 0) flag = 1;
									else printf(", ");

									printf("%s", sql_row[0]); // 고객 이름을 한 튜플씩 받아온다.
								}
								printf(".\n");
								mysql_free_result(sql_result);
							}

							printf("\n");
						}
						else if (choice == 2) {
							printf("\n---- TYPE I-2 ----\n");
							printf("** Find all recipients who had a package on the truck at the time of the crach. **\n");

							//쿼리 넣고 수취인 출력

							query = "select shipping_name from customer, package, trace where customer.customer_ID = package.customer_ID and package.tracking_number = trace.tracking_number and t_date = '2020-03-07' and license_plate = '1721' and state = 'in transit'";
							flag = 0;
							state = 0;
							state = mysql_query(connection, query);
							if (state == 0)
							{
								printf("Recipient name : ");

								sql_result = mysql_store_result(connection);
								while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
								{
								
									if (flag == 0) flag = 1;
									else printf(", ");

									printf("%s", sql_row[0]); // 고객 이름을 한 튜플씩 받아온다.
								}
								printf(".\n");
								mysql_free_result(sql_result);
							}

							printf("\n");
						}
						else if (choice == 3) {
							printf("\n---- TYPE I-3 ----\n");
							printf("** Find the last successful delivery by that truck prior to the crash. **\n");

							//쿼리 넣고 송장번호 출력

							query = "select mt.tracking_number from trace mt where mt.t_time = (select max(st.t_time) from customer, package, trace st where customer.customer_ID = package.customer_ID and package.tracking_number = st.tracking_number and st.t_date = '2020-03-07' and st.license_plate = '1721' and st.state = 'Delivered'); ";
							flag = 0;
							state = 0;
							state = mysql_query(connection, query);
							if (state == 0)
							{
								printf("the last successful delivery : ");

								sql_result = mysql_store_result(connection);
								sql_row = mysql_fetch_row(sql_result);
								printf("%s.\n", sql_row[0]);

								mysql_free_result(sql_result);
							}

							printf("\n");
						}
					}
					if (choice == 0) break;
				}
				break;
			case 2:
				printf("---- TYPE II ----\n");
				while (1) {
					printf("** Find the customer who has shipped the most packages in certain year **\n");
					printf("Which year? >");	// 년도 입력   
					scanf("%d", &year); 

					if (year == 0) break;


					//쿼리에 year 넣고 이름 출력

					qu[0] = '\0';
					sprintf(qu, "select c_name from bill b, payment p, customer c where c.customer_ID = b.customer_ID and year(date_of_delivery) = '%d' and b.bill_number = p.bill_number group by c.customer_ID having count(*) = (select max(cnt) from(select bb.customer_ID, count(tracking_number) cnt from bill bb, payment pp where year(date_of_delivery) = '%d' and bb.bill_number = pp.bill_number group by bb.customer_ID) a)", year, year);
					//입력받은 값을 넣어 쿼리문을 수정한다.

					flag = 0;
					state = 0;
					state = mysql_query(connection, qu);
					if (state == 0)
					{

						sql_result = mysql_store_result(connection);
						sql_row = mysql_fetch_row(sql_result);
						if (sql_row == NULL) { // 결과 테이블이 비어있을 경우
							printf("Not exist!\n");
							mysql_free_result(sql_result);
							continue;
						}
						printf("Customer name : "); // 결과 출력
						printf("%s.\n", sql_row[0]);

						mysql_free_result(sql_result);
					}

					printf("\n");

				}
				break;
			case 3:
				printf("---- TYPE III ----\n");
				while (1) {
					printf("** Find the customer who has spent the most money on shipping in past year **\n");
					printf("Which year? >");
					scanf("%d", &year);

					if (year == 0) break;


					//쿼리에 year 넣고 이름 출력

					qu[0] = '\0';
					sprintf(qu, "select c_name from bill b, payment p, customer c where c.customer_ID = b.customer_ID and year(date_of_delivery) = '%d' and b.bill_number = p.bill_number group by c.customer_ID having sum(amount_of_money) = (select max(s) from(select bb.customer_ID, sum(amount_of_money) s from bill bb, payment pp where year(date_of_delivery) = '%d' and bb.bill_number = pp.bill_number group by bb.customer_ID) a)", year, year);
					//입력받은 값을 넣어 쿼리문을 수정한다.
					flag = 0;
					state = 0;
					state = mysql_query(connection, qu);
					if (state == 0)
					{
						sql_result = mysql_store_result(connection);
						sql_row = mysql_fetch_row(sql_result);
						if (sql_row == NULL) { // 결과 테이블이 비어있을 경우
							printf("Not exist!\n");
							mysql_free_result(sql_result);
							continue;
						}
						printf("Customer name : "); // 결과 출력
						printf("%s.\n", sql_row[0]);

						mysql_free_result(sql_result);
					}

					printf("\n");


				}
				break;
			case 4:
				printf("---- TYPE IV ----\n");
				printf("** Find those packages that were not delivered within the promised time **\n");
				//쿼리 넣고 출력

				query = "select p.tracking_number from trace t, package p where state = 'Delivered' and t.tracking_number = p.tracking_number and p.estimated_delivery < t.t_date";

				flag = 0;
				state = 0;
				state = mysql_query(connection, query);
				if (state == 0)
				{
					printf("Late delivery : ");

					sql_result = mysql_store_result(connection);
					while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
					{
						if (flag == 0) flag = 1;
						else printf(", ");

						printf("%s", sql_row[0]); // 결과 출력
					}
					printf(".\n");
					mysql_free_result(sql_result);
				}

				printf("\n");
				break;
			case 5:
				printf("---- TYPE V ----\n");
				printf("** Generate the bill for each customer for the past certain month **\n");
				printf("Customer Name > "); // 이름 입력
				scanf("%s", name);

				if (name[0] == '0') break;

				printf("Which month(YYYY-MM)? > "); // 년도 및 월 입력
				scanf("%d-%d", &year, &month);

				if (year == 0) break;

				//영수증만들기. 파일입출력.
				char filename[50];
				sprintf(filename, "[bill]%d_%02d_%s.txt", year, month, name);
				FILE* fw = fopen(filename, "w");


				qu[0] = '\0';
				sprintf(qu, "select c_name, home_address, sum(amount_of_money) s from bill, payment, customer where year(date_of_delivery)='%d' and month(date_of_delivery)='%02d' and c_name = '%s' and customer.customer_ID = bill.customer_ID and bill.bill_number = payment.bill_number group by customer.customer_ID", year, month, name);
				//입력받은 값을 넣어 쿼리문을 수정한다.

				flag = 0;
				state = 0;
				state = mysql_query(connection, qu);
				if (state == 0) // 간단한 영수증 [ 이름 / 청구 주소 / 청구 금액 ]
				{
					sql_result = mysql_store_result(connection);
					sql_row = mysql_fetch_row(sql_result);
					if (sql_row == NULL) { // 결과 테이블이 비어있을 경우
						printf("Not exist!\n");
						mysql_free_result(sql_result);
						fclose(fw);
						continue;
					}

					fprintf(fw, "bill--------------------------------\n");
					fprintf(fw, "Customer\t\tAddress\t\tAmount\n");
					fprintf(fw, "%s\t\t%s\t\t%s\\\n", sql_row[0], sql_row[1], sql_row[2]);
					fprintf(fw, "------------------------------------\n");

					mysql_free_result(sql_result);
				}

				qu[0] = '\0';
				sprintf(qu, "select pk.tracking_number, p_type from package pk where pk.tracking_number in (select p.tracking_number from bill b, customer c, payment p where p.bill_number = b.bill_number and b.customer_ID = c.customer_ID and year(date_of_delivery)='%d' and month(date_of_delivery)='%02d' and c_name = '%s') order by p_type", year, month, name);
				flag = 0;
				state = 0;
				state = mysql_query(connection, qu);
				if (state == 0) // 택배 유형별로 정렬된 영수증 [ 택배 번호 / 타입 ]
				{
					sql_result = mysql_store_result(connection);

					fprintf(fw, "type list---------------------------\n");
					fprintf(fw, "tracking_number\ttype\n");
					while ((sql_row = mysql_fetch_row(sql_result)) != NULL) {
						fprintf(fw, "%s\t\t%s\n", sql_row[0], sql_row[1]);
					}
					fprintf(fw, "------------------------------------\n");

					mysql_free_result(sql_result);
				}

				qu[0] = '\0';
				sprintf(qu, "select pk.tracking_number, p_type,p.amount_of_money, date_of_delivery, p_time, handle_with_care from bill b, customer c, payment p, package pk, trace t where p.bill_number = b.bill_number and b.customer_ID = c.customer_ID and pk.tracking_number = p.tracking_number and t.tracking_number = pk.tracking_number and state = 'Delivered' and year(date_of_delivery)='%d' and month(date_of_delivery)='%02d' and c_name = '%s'", year, month, name);

				flag = 0;
				state = 0;
				state = mysql_query(connection, qu);
				if (state == 0) // [ 송장 번호 / 개별 금액 / 결제 날짜 / 시간 / 취급주의여부]
				{
					sql_result = mysql_store_result(connection);

					fprintf(fw, "Itemized----------------------------\n");
					fprintf(fw, "tracking_number\ttype\t\tamount\t\tdate of delivery\ttime\t\thandle with care\n");
					while ((sql_row = mysql_fetch_row(sql_result)) != NULL) {
						fprintf(fw, "%s\t\t%s\t\t%s\\\t\t%s\t%s\t\t%s\n", sql_row[0], sql_row[1], sql_row[2], sql_row[3], sql_row[4], sql_row[5]);
					}
					fprintf(fw, "------------------------------------\n");

					mysql_free_result(sql_result);
				}
				fclose(fw);

				if (state == 0) printf("\nGeneration Completed\n\n");

				break;
			default:;

			}
		}

		//===================================== drop

		for (int i = 0; i < d_idx; i++) {
			mysql_query(connection, drop[i]);
		}

		mysql_close(connection);
	}

	return 0;
}