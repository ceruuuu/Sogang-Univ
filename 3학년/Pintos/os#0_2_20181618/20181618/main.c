#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "list.h"
#include "hash.h"
#include "bitmap.h"
#include "round.h"
#include "limits.h"
#include "hex_dump.h"

int objNum;
struct oobj *obj[100];

int search_index(char *input){

	for(int i=0; i<objNum;i++){
		if(strcmp(obj[i]->name, input)==0) return i;
	
	}
	return -1; //일치하지않을때
}

bool list_less(const struct list_elem *a, const struct list_elem *b, void *aux){
	struct list_item *l_item1, *l_item2;
	int a_num, b_num;
	
	l_item1 = list_entry(a, struct list_item, elem);
	a_num = l_item1->data;
	l_item2 = list_entry(b, struct list_item, elem);
	b_num = l_item2->data;

	if(a_num < b_num) return true;
	else return false;
}

unsigned hash_func(const struct hash_elem *e, void *aux){
	struct hash_item *h_item;
	h_item = hash_entry(e, struct hash_item, elem);
	return hash_int(h_item->data);
}

bool h_less_func(const struct hash_elem *a, const struct hash_elem *b, void *aux){
	struct hash_item *h_item1, *h_item2;
	
	h_item1 = hash_entry(a, struct hash_item, elem);
	h_item2 = hash_entry(b, struct hash_item, elem);
	
	if(h_item1->data < h_item2->data) return true;
	else return false;
}

void h_action_square(struct hash_elem *e, void *aux){
	struct hash_item *h_item;

	h_item = hash_entry(e, struct hash_item, elem);
	h_item->data = (h_item->data)*(h_item->data);
}

void h_action_triple(struct hash_elem *e, void *aux){
	struct hash_item *h_item;
	
	h_item = hash_entry(e, struct hash_item, elem);
	h_item->data = (h_item->data)*(h_item->data)*(h_item->data);
}

void h_action_free(struct hash_elem *e, void *aux){
	free(e);
}

int main(){
	
	char inst[100];
	char *ptr;
	char temp[50];
	int idx,idx2, num1, num2, num3;
	int i,len;

	struct list_item *l_item;
	struct list *l_list, *l_list2;
	struct list_elem *l_elem1, *l_elem2, *l_elem3;

	struct hash_elem *h_elem1;
	struct hash_iterator *h_iter;
	struct hash_item *h_item;
	objNum = 0;

	while(1){
		fgets(inst, 100, stdin);
		
		inst[strlen(inst)] = '\0';	
	
		if(strncmp(inst, "create", 6) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			if(strcmp(ptr, "list") == 0){
				ptr = strtok(NULL, " ");
				obj[objNum] = (struct oobj*)malloc(sizeof(struct oobj));
				strcpy(obj[objNum]->name, ptr);
				len = strlen(obj[objNum]->name);
				obj[objNum]->name[len-1] = '\0';

				obj[objNum]->LL = (struct list*)malloc(sizeof(struct list));
				list_init(obj[objNum++]->LL);
			}
			else if(strcmp(ptr, "bitmap") == 0){
				ptr = strtok(NULL, " ");
				obj[objNum] = (struct oobj*)malloc(sizeof(struct oobj));
				strcpy(obj[objNum]->name, ptr);
				ptr = strtok(NULL, " ");
				num1 = atoi(ptr);

				obj[objNum++]->BB = bitmap_create(num1);
			}
			else if(strcmp(ptr, "hashtable") == 0){
				ptr = strtok(NULL, " ");
				obj[objNum] = (struct oobj*)malloc(sizeof(struct oobj));
				ptr[strlen(ptr)-1] = '\0';
				strcpy(obj[objNum]->name, ptr);
				obj[objNum]->HH = (struct hash*)malloc(sizeof(struct hash));
				hash_init(obj[objNum++]->HH, hash_func, h_less_func, NULL);
			}
		}
		else if(strncmp(inst, "delete", 6) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';

			if(ptr[0] == 'l'){		
				idx = search_index(ptr);
				l_list = obj[idx]->LL;
				num1 = list_size(l_list);
			
				for(i=0;i<num1;i++){
					list_pop_front(l_list);
				}

				strcpy(obj[idx]->name, "");
				free(obj[idx]->LL);	
			}
			else if(ptr[0] == 'b'){
				idx = search_index(ptr);
				bitmap_destroy(obj[idx]->BB);
				strcpy(obj[idx]->name, "");
			}
			else if(ptr[0] == 'h'){
				idx = search_index(ptr);
				hash_destroy(obj[idx]->HH,h_action_free);
				strcpy(obj[idx]->name, "");
				free(obj[idx]->HH);
			}
		}
		else if(strncmp(inst, "dumpdata", 8)==0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);

			if(ptr[0] == 'l'){
				l_list = obj[idx]->LL;
				l_elem1 = list_begin(l_list);
	
				if(l_elem1 == list_end(l_list)) continue;
				while(l_elem1 != list_end(l_list)){
					l_item = list_entry(l_elem1,struct list_item, elem);
					printf("%d ", l_item->data);
	
					l_elem1 = list_next(l_elem1);
				}
				printf("\n");
			}
			else if(ptr[0] == 'b'){
				num1 = bitmap_size(obj[idx]->BB);
				
				for(i=0;i<num1;i++){
					printf("%d",bitmap_test(obj[idx]->BB, i));
				}
				printf("\n");
	
			}
			else if(ptr[0] == 'h'){
				h_iter = (struct hash_iterator*)malloc(sizeof(struct hash_iterator));
				hash_first(h_iter, obj[idx]->HH);
				h_elem1 = h_iter->elem;
				num1 = hash_size(obj[idx]->HH);

				for(i=0;i<num1;i++){
					h_elem1 = hash_next(h_iter);
					h_iter->elem = h_elem1;
					h_item = hash_entry(h_elem1, struct hash_item, elem);
					printf("%d ", h_item->data);
				}
				if(num1 != 0) printf("\n");
			}
		}
		else if(strncmp(inst, "list_push_front", 15)==0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
	
			idx = search_index(ptr);
	
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);

			l_item = (struct list_item*)malloc(sizeof(struct list_item));
			l_item->data = num1;
			
			list_push_front(obj[idx]->LL, &l_item->elem);
		}
		else if(strncmp(inst, "list_push_back", 14)==0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);

			l_item = (struct list_item*)malloc(sizeof(struct list_item));
			l_item->data = num1;
			
			list_push_back(obj[idx]->LL, &l_item->elem);
		}
		else if(strncmp(inst, "list_insert ", 12) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);

			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);
			ptr = strtok(NULL, " ");
			num2 = atoi(ptr);
			
			l_list = obj[idx]->LL;
			l_elem1 = list_begin(l_list);
	
			l_item = (struct list_item*)malloc(sizeof(struct list_item));
			l_item->data = num2;
			
			if(num1 == 0){	
				list_push_front(obj[idx]->LL, &l_item->elem);
			}
			else{
				for(i=0;i<num1;i++){
					l_elem1 = list_next(l_elem1);
				}
				list_insert(l_elem1, &l_item->elem);
			}

		}
		else if(strncmp(inst, "list_insert_ordered", 19) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);

			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);

			l_list = obj[idx]->LL;
			l_elem1 = list_begin(l_list);

			l_item = (struct list_item*)malloc(sizeof(struct list_item));
			l_item->data = num1;

			list_insert_ordered(l_list, &l_item->elem,list_less ,NULL);
		}
		else if(strncmp(inst, "list_pop_front", 14)==0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);
			
			list_pop_front(obj[idx]->LL);
		}
		else if(strncmp(inst, "list_pop_back", 13)==0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);

			list_pop_back(obj[idx]->LL);
		}
		else if(strncmp(inst, "list_remove", 11) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);

			l_elem1 = list_begin(obj[idx]->LL);

			if(num1 == 0){	
				list_pop_front(obj[idx]->LL);
			}
			else{
				for(i=0;i<num1;i++){
					l_elem1 = list_next(l_elem1);
				}
				list_remove(l_elem1);
			}	
		}
		else if(strncmp(inst, "list_empty", 10) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);
			
			if(list_empty(obj[idx]->LL)==1) printf("true\n");
			else printf("false\n");
		}
		else if(strncmp(inst, "list_front", 10) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);

			l_elem1 = list_front(obj[idx]->LL);
			l_item = list_entry(l_elem1, struct list_item, elem);
			printf("%d\n", l_item->data);
		}
		else if(strncmp(inst, "list_back", 9) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);

			l_elem1 = list_back(obj[idx]->LL);
			l_item = list_entry(l_elem1, struct list_item, elem);
			printf("%d\n", l_item->data);		
		}
		else if(strncmp(inst, "list_size", 9) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);
			
			printf("%zu\n", list_size(obj[idx]->LL));
		}
		else if(strncmp(inst, "list_max", 7) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);
			
			l_elem1 = list_max(obj[idx]->LL, list_less, NULL);
			l_item = list_entry(l_elem1, struct list_item, elem);
			printf("%d\n", l_item->data);
		}
		else if(strncmp(inst, "list_min", 7) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);
			
			l_elem1 = list_min(obj[idx]->LL, list_less, NULL);
			l_item = list_entry(l_elem1, struct list_item, elem);
			printf("%d\n", l_item->data);
		}
		else if(strncmp(inst, "list_reverse", 12) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);
			
			list_reverse(obj[idx]->LL);
		}
		else if(strncmp(inst, "list_sort", 9) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);

			list_sort(obj[idx]->LL, list_less, NULL);
		}
		else if(strncmp(inst, "list_splice", 11) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);
			ptr = strtok(NULL, " ");
			idx2 = search_index(ptr);
			ptr = strtok(NULL, " ");
			num2 = atoi(ptr);
			ptr = strtok(NULL, " ");
			num3 = atoi(ptr);

			l_elem1 = list_begin(obj[idx]->LL);
			for(i=0;i<num1;i++){
				l_elem1 = list_next(l_elem1);
			}
			
			l_elem2 = list_begin(obj[idx2]->LL);
			for(i=0;i<num2;i++){
				l_elem2 = list_next(l_elem2);
			}

			l_elem3 = list_begin(obj[idx2]->LL);
			for(i=0;i<num3;i++){
				l_elem3 = list_next(l_elem3);
			}

			list_splice(l_elem1, l_elem2, l_elem3);

		}
		else if(strncmp(inst, "list_unique", 11) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			strcpy(temp, ptr);
			
			ptr = strtok(NULL, " ");

			if(ptr == NULL){ // 중복삭제
				temp[strlen(temp)-1] = '\0';
				idx = search_index(temp);
			
				l_list = obj[idx]->LL;
				list_unique(l_list, NULL, list_less, NULL);
			}
			else{
				idx = search_index(temp);
				
				ptr[strlen(ptr)-1] = '\0';
				idx2 = search_index(ptr);

				l_list = obj[idx]->LL;
				l_list2 = obj[idx2]->LL;

				list_unique(l_list, l_list2, list_less, NULL);
			}
		}
		else if(strncmp(inst, "list_swap", 9) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);
			ptr = strtok(NULL, " ");
			num2 = atoi(ptr);

			l_elem1 = list_begin(obj[idx]->LL);
			for(i=0;i<num1;i++){
				l_elem1 = list_next(l_elem1);
			}
		
			l_elem2 = list_begin(obj[idx]->LL);
			for(i=0;i<num2;i++){
				l_elem2 = list_next(l_elem2);
			}
			
			list_swap(l_elem1, l_elem2);

		}
		else if(strncmp(inst, "list_shuffle", 12) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);

			list_shuffle(obj[idx]->LL);
		}
		else if(strncmp(inst, "hash_insert", 11) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);

			h_item = (struct hash_item*)malloc(sizeof(struct hash_item));
			h_item->data = num1;

			hash_insert(obj[idx]->HH, &(h_item->elem));

		}
		else if(strncmp(inst, "hash_replace", 12) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);

			h_item = (struct hash_item*)malloc(sizeof(struct hash_item));
			h_item->data = num1;

			hash_replace(obj[idx]->HH, &(h_item->elem));

		}
		else if(strncmp(inst, "hash_delete", 11) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);
					
			h_iter = (struct hash_iterator*)malloc(sizeof(struct hash_iterator));
			hash_first(h_iter, obj[idx]->HH);
			h_elem1 = h_iter->elem;
			num2 = hash_size(obj[idx]->HH);

			for(i=0;i<num2;i++){
				h_elem1 = hash_next(h_iter);
				h_iter->elem = h_elem1;
				h_item = hash_entry(h_elem1, struct hash_item, elem);
				if(h_item->data == num1){
					hash_delete(obj[idx]->HH, h_elem1);
					break;
				}
			}
		}
		else if(strncmp(inst, "hash_apply", 10) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';

			if(strcmp(ptr, "square") == 0)
				hash_apply(obj[idx]->HH, h_action_square);		
			if(strcmp(ptr, "triple") == 0)
				hash_apply(obj[idx]->HH, h_action_triple);		
		
		}
		else if(strncmp(inst, "hash_empty", 10) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);
			
			if(hash_empty(obj[idx]->HH)==1) printf("true\n");
			else printf("false\n");
		}
		else if(strncmp(inst, "hash_size", 9) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);
			
			printf("%zu\n", hash_size(obj[idx]->HH));
		}
		else if(strncmp(inst, "hash_clear", 10) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);
			
			hash_clear(obj[idx]->HH, h_action_free);
		}
		else if(strncmp(inst, "hash_find", 9) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);

			h_item = (struct hash_item*)malloc(sizeof(struct hash_item));
			h_item->data = num1;
			h_elem1 = hash_find(obj[idx]->HH, &(h_item->elem));
			if(h_elem1 != NULL) printf("%d\n", num1);
			else free(h_item);
		}
		else if(strncmp(inst, "bitmap_dump", 11) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);

			bitmap_dump(obj[idx]->BB);

		}
		else if(strncmp(inst, "bitmap_set ", 11) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			
			if(strcmp(ptr, "true") == 0)
				bitmap_set(obj[idx]->BB, num1, true);
			else
				bitmap_set(obj[idx]->BB, num1, false);
			
		}
		else if(strncmp(inst, "bitmap_mark", 11) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);		

			bitmap_mark(obj[idx]->BB, num1);
		}
		else if(strncmp(inst, "bitmap_set_multiple", 19) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);
			ptr = strtok(NULL, " ");
			num2 = atoi(ptr);
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			
			if(strcmp(ptr, "true") == 0)
				bitmap_set_multiple(obj[idx]->BB, num1, num2, true);
			else
				bitmap_set_multiple(obj[idx]->BB, num1, num2, false);
		}
		else if(strncmp(inst, "bitmap_reset", 12) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);		

			bitmap_reset(obj[idx]->BB, num1);
		}
		else if(strncmp(inst, "bitmap_set_all", 14) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';		

			if(strcmp(ptr, "true")==0)
				bitmap_set_all(obj[idx]->BB, true);
			else
				bitmap_set_all(obj[idx]->BB, false);
		}
		else if(strncmp(inst, "bitmap_flip", 11) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);		
			
			bitmap_flip(obj[idx]->BB, num1);
		}
		else if(strncmp(inst, "bitmap_test", 11) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);		
			
			if(bitmap_test(obj[idx]->BB, num1)==1)
				printf("true\n");
			else
				printf("false\n");
		}
		else if(strncmp(inst, "bitmap_count", 12) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);
			ptr = strtok(NULL, " ");
			num2 = atoi(ptr);
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			
			if(strcmp(ptr, "true") == 0)
				printf("%zu\n", bitmap_count(obj[idx]->BB, num1, num2, true));
			else
				printf("%zu\n", bitmap_count(obj[idx]->BB, num1, num2, false));
		}
		else if(strncmp(inst, "bitmap_size", 11) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			idx = search_index(ptr);
			
			printf("%zu\n", bitmap_size(obj[idx]->BB));
		}
		else if(strncmp(inst, "bitmap_scan ", 12) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);
			ptr = strtok(NULL, " ");
			num2 = atoi(ptr);
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			
			if(strcmp(ptr, "true") == 0)
				printf("%zu\n", bitmap_scan(obj[idx]->BB, num1, num2, true));
			else
				printf("%zu\n", bitmap_scan(obj[idx]->BB, num1, num2, false));
		}
		else if(strncmp(inst, "bitmap_scan_and_flip", 20) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);
			ptr = strtok(NULL, " ");
			num2 = atoi(ptr);
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';
			
			if(strcmp(ptr, "true") == 0)
				printf("%zu\n", bitmap_scan_and_flip(obj[idx]->BB, num1, num2, true));
			else
				printf("%zu\n", bitmap_scan_and_flip(obj[idx]->BB, num1, num2, false));
		}
		else if(strncmp(inst, "bitmap_none", 11) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);
			ptr = strtok(NULL, " ");
			num2 = atoi(ptr);
		
			if(bitmap_none(obj[idx]->BB, num1, num2) == 1)
				printf("true\n");
			else
				printf("false\n");
		}
		else if(strncmp(inst, "bitmap_all", 10) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);
			ptr = strtok(NULL, " ");
			num2 = atoi(ptr);
		
			if(bitmap_all(obj[idx]->BB, num1, num2) == 1)
				printf("true\n");
			else
				printf("false\n");
		}
		else if(strncmp(inst, "bitmap_any", 10) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);
			ptr = strtok(NULL, " ");
			num2 = atoi(ptr);
		
			if(bitmap_any(obj[idx]->BB, num1, num2) == 1)
				printf("true\n");
			else
				printf("false\n");
		}
		else if(strncmp(inst, "bitmap_contains", 15) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);
			ptr = strtok(NULL, " ");
			num2 = atoi(ptr);
			ptr = strtok(NULL, " ");
			ptr[strlen(ptr)-1] = '\0';

			if(strcmp(ptr, "true")==0){
				if(bitmap_contains(obj[idx]->BB, num1, num2, true) == 1)
					printf("true\n");
				else
					printf("false\n");
			}
			else{
				if(bitmap_contains(obj[idx]->BB, num1, num2, false) == 1)
					printf("true\n");
				else
					printf("false\n");
			}
		}
		else if(strncmp(inst, "bitmap_expand", 13) == 0){
			ptr = strtok(inst, " ");
			ptr = strtok(NULL, " ");
			idx = search_index(ptr);
			ptr = strtok(NULL, " ");
			num1 = atoi(ptr);		
			
			obj[idx]->BB = bitmap_expand(obj[idx]->BB, num1);
		}
		else if(strcmp(inst, "quit\n") == 0) break;
		
	}

	return 0;
}
