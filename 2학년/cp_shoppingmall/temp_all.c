#include "all.h"

int CountReadfile(FILE* fpr){
    int n=0;
    char t[10000];

    fgets(t,sizeof(t),fpr);

    while(fgets(t,sizeof(t),fpr) != NULL){
        n++;
    }

    return n;
}//저장된 갯수 세는 함수.

void Customer_Readfile(FILE *fpr, customer *a,int n){
    int i;
    char t[10000];
    char *str;

    fgets(t, sizeof(t), fpr); // 첫 줄 없애기

    for(i=0;i<n;i++){
        fgets(t, sizeof(t), fpr);
        if(t == NULL) break;
        else{
            str = strtok(t, ",");
            a[i].number = atoi(str);
            strcpy(a[i].id, strtok(NULL, ","));
            strcpy(a[i].pw, strtok(NULL, ","));
            strcpy(a[i].name, strtok(NULL, ",")); 
            strcpy(a[i].address, strtok(NULL, ","));
            str = strtok(NULL, ",");
            a[i].balance = atoi(str);
            str = strtok(NULL, "\n");
            a[i].state = atoi(str);
        }
    }

}

void Product_Readfile(FILE *fpr, product *b, int n){
    char t[10000];
    int i;
    char *str;

    fgets(t, sizeof(t), fpr);

    for(i=0;i<n;i++){
        fgets(t, sizeof(t), fpr);
        if(t == NULL) break;
        else{
            str = strtok(t, ",");
            b[i].number = atoi(str);
            strcpy(b[i].p_name, strtok(NULL, ","));
            strcpy(b[i].category, strtok(NULL, ","));
            str = strtok(NULL, ",");
            b[i].price = atoi(str);
            str = strtok(NULL, "\n");
            b[i].state = atoi(str);
        }
    }
}

void History_Readfile(FILE *fpr, history *c, int n){
    char t[10000];
    int i;
    char *str;

    fgets(t, sizeof(t), fpr);

    for(i=0;i<n;i++){
        fgets(t, sizeof(t), fpr);
        if(t==NULL) break;
        else{
            str = strtok(t, ",");
            c[i].order_n = atoi(str);
            str = strtok(NULL, ",");
            c[i].product_n = atoi(str);
            str = strtok(NULL, ",");
            c[i].customer_n = atoi(str);
            str = strtok(NULL, ",");
            c[i].state = atoi(str); 
        }
    }
}

void Customer_Signup(){
    FILE* fpw;
    FILE* fpr;
    int newidflag=0;
    int newpwflag=0;
    char newname[20];
    char newid[50];
    char newpw[50];
    char newpw_re[50];
    char newadd[100];
    int i,j,k,n;

    fpr = fopen("customer.csv","r");
    n = CountReadfile(fpr);
    fclose(fpr);

    customer *a;
    a = (customer*)malloc(sizeof(customer)*n);

    fpr = fopen("customer.csv","r");
    Customer_Readfile(fpr,a,n);
    fclose(fpr);

    fpw = fopen("customer.csv","w");

    printf("===========회원 가입 페이지===========\n");
    printf(" 이름 : ");

    getchar();
    fgets(newname, sizeof(newname), stdin);
    newname[strlen(newname)-1] = '\0';

    while(newidflag == 0){
        printf(" **아이디와 비밀번호에는 공백을 포함하지 않습니다.\n");
        printf(" 아이디 : ");
        scanf("%s", newid);
        //아이디 다른회원과 중복확인
        for(i=0;i<n;i++){
            if(strcmp(newid, a[i].id) == 0){
                printf("\n 중복된 아이디가 이미 존재합니다.\n 다른 아이디를 입력해 주십시오.\n");
                break;
            }
            else{
                newidflag = 1;
                printf(" 사용가능한 아이디입니다.\n");
                break;
            }
        }
    }
    while(newpwflag == 0){
        printf(" 비밀번호 : ");
        scanf("%s", newpw);
        printf(" 비밀번호 확인 : ");
        scanf("%s", newpw_re);
        //비밀번호같은가확인
        if (strcmp(newpw, newpw_re) == 0) {
            newpwflag=1;
            printf(" 비밀번호 확인이 완료되었습니다.\n");
        }
        else printf("\n 비밀번호를 한 번 더 확인해 주십시오.\n");
    }

    printf(" 주소 : ");
    getchar();
    fgets(newadd, sizeof(newadd), stdin);
    newadd[strlen(newadd)-1] = '\0';

    if(newidflag == 1 && newpwflag == 1){
        //        getchar();
        fprintf(fpw, "회원번호, 아이디, 비밀번호, 이름, 주소, 잔액, 상태\n");
        for(i=0;i<n;i++){
            fprintf(fpw,"%d,%s,%s,%s,%s,%d,%d\n",i+1, a[i].id, a[i].pw, a[i].name, a[i].address, a[i].balance, a[i].state);
        }
        fprintf(fpw, "%d,%s,%s,%s,%s,%d,%d",n+1, newid, newpw, newname, newadd,0,1);
    } //회원가입 가능
    fclose(fpw);
}


void customer_inf_print(){
    int num,i;
    customer* a;        
    FILE* fpr;

    fpr = fopen("customer.csv","r");
    num = CountReadfile(fpr);        
    fclose(fpr);        

    a= (customer*)malloc(sizeof(customer)*num);

    fpr = fopen("customer.csv","r");
    Customer_Readfile(fpr,a,num);
    fclose(fpr);


    printf("=====모든 회원 정보=====\n");
    printf(" 회원번호 / 아이디 / 비밀번호 / 이름 / 주소 / 잔액\n");
    for(i=0;i<num;i++){
        if(a[i].state == 1){
            printf(" %d번 / %s / %s / %s / %s / %d원 \n", a[i].number, a[i].id, a[i].pw, a[i].name, a[i].address, a[i].balance);
        }
    }
    //    printf(" Enter\n");
    //    getchar();
}

int customer_interface(char*);

int Customer_login(customer *a_1){
    FILE* fpr;
    int n,i,p,j;
    int loginFlag=0;
    char inputid[50];
    char inputpw[50];
    customer *a;
    fpr = fopen("customer.csv","r");
    n = CountReadfile(fpr);
    fclose(fpr);

    a = (customer*)malloc(sizeof(customer)*n);
    fpr = fopen("customer.csv","r");
    Customer_Readfile(fpr,a,n);
    fclose(fpr);

    for(i=0;i<3;i++){
        printf("=====회원 로그인 페이지=====\n");
        printf(" 아이디 : ");
        scanf("%s", inputid);
        printf(" 비밀번호 : ");
        scanf("%s", inputpw);
        for(j=0;j<n;j++){
            if(strcmp(inputid, a[j].id) == 0 && strcmp(inputpw, a[j].pw) == 0){
                //로그인 성공
                loginFlag = 1;
                *a_1 = a[j]; 
                return a[j].state;//1이면 존재하는 회원 -1이면 탈퇴회원
            }
        }
        if(loginFlag == 0){
            printf(" 아이디 또는 비밀번호가 일치하지 않습니다.\n 다시 입력해주십시오.\n");
        }
    }
    printf(" 로그인에 3회 실패하였습니다. 시작화면으로 돌아갑니다.\n Enter\n");
    getchar();
    return -2;//없는 아이디

    //파일 여는거 위주로
}


int Manager_login(){
    char inputid[50];
    char inputpw[50];
    int i;
    for(i=0;i<3;i++){
        printf("=====관리자 로그인=====\n");
        printf(" 아이디 : ");
        scanf("%s", inputid);
        printf(" 비밀번호 : ");
        scanf("%s", inputpw);

        if(strcmp(inputid,"day6is") == 0 && strcmp(inputpw,"superband" ) == 0){
            return 1;
            //로그인 성공
        }
        else if(strcmp(inputid,"admin") == 0 && strcmp(inputpw,"password" ) == 0){
            return 1;
            //로그인 성공
        }

        printf(" 아이디 또는 비밀번호가 일치하지 않습니다.\n 다시 입력해주십시오.\n");
        return 0;
        //로그인실패
    }
}

//회원정보수정하기


int customer_interface(char *name){
    int choose_n;
    printf("=====%s님, Day6 Would에 오신 것을 환영합니다=====\n", name);
    printf(" 1. 회원 정보 조회 및 수정\n");
    printf(" 2. 가상계좌 입금 및 출금\n");
    printf(" 3. 상품 검색\n");
    printf(" 4. 구매 내역 조회\n");
    printf(" 5. 배송 내역 조회\n");
    printf(" 6. 로그아웃 및 종료\n");
    printf("  선택 > ");
    scanf("%d", &choose_n);
    return choose_n;
}

int manager_interface(){
    int choose_n;
    printf("=====Day6 Would 관리자 페이지=====\n");
    printf(" 1. 회원 정보 조회 및 수정\n");
    printf(" 2. 상품 등록 및 삭제\n");
    printf(" 3. 상품 정보 수정\n");
    printf(" 4. 상품 조회 및 통계 보기\n");
    printf(" 5. 판매 내역 및 통계 보기\n");
    printf(" 6. 배송 내역 조회 및 수정\n");
    printf(" 7. 로그아웃 및 종료\n");
    printf("  선택 > ");
    scanf("%d", &choose_n);
    return choose_n;
}

int main_interface(){
    int choose_n;
    printf("=====Day6 Would에 오신 것을 환영합니다.=====\n");
    printf(" 1. 회원 로그인\n");
    printf(" 2. 회원 가입\n");
    printf(" 3. 상품 검색\n");
    printf(" 4. 관리자 로그인\n");
    printf(" 5. 종료\n");
    printf(" 선택 > ");
    scanf("%d",&choose_n);
    return choose_n;
}


void print_customer_inf(customer a_1){
    printf("===회원 정보 조회 페이지===\n");
    printf(" 1. 아이디 : %s\n", a_1.id);
    printf(" 2. 비밀번호 : ******\n");
    printf(" 3. 이름 : %s\n", a_1.name);
    printf(" 4. 주소 : %s\n", a_1.address);
    printf("==========================\n");
}


int pw_check(customer a_1){
    char t[100];

    printf(" 비밀번호를 한 번 더 입력해 주십시오.\n > ");
    scanf("%s",t);
    if(strcmp((a_1).pw,t)==0){ 
        return 1;
    }
    else{
        printf(" 비밀번호가 일치하지 않습니다.\n");
        return 0;
    }

}


customer m_customer_update(customer *a, int customer_number){
    FILE* fpr;
    int n;
    int flag = 0;
    int choose_n3,i = 0;
    char t[1000];
    int temp;
    customer a_1;

    fpr = fopen("customer.csv","r");
    n = CountReadfile(fpr);
    fclose(fpr);
    //    a = (customer*)malloc(sizeof(customer)*n);

    fpr = fopen("customer.csv","r");
    Customer_Readfile(fpr,a,n);
    fclose(fpr);

    while(a[i].number != customer_number){
        i++;   
    }

    printf(" 1. 아이디 수정\n 2. 비밀번호 수정\n 3. 이름 수정\n 4. 주소 수정\n 5. 잔액 수정\n 6. 탈퇴 여부 수정\n");

    printf(" 선택 > ");
    scanf("%d",&choose_n3);

    if(choose_n3==1){
        printf(" 변경할 아이디를 입력해주십시오.\n >> ");
        scanf("%s", t);
        while(flag == 0){
            if(strcmp(a[i].id, t) == 0){
                printf(" 기존의 아이디와 일치합니다. 다시 입력해주십시오.\n > ");
                scanf("%s", t);
            }
            else{
                strcpy(a[i].id, t);
                flag = 1;
            }
        }
    }
    if(choose_n3 ==2){
        printf(" 변경할 비밀번호를 입력해주십시오.\n >> ");
        scanf("%s", t); 

        while(flag == 0){
            if(strcmp(a[i].pw, t) == 0){
                printf(" 기존의 비밀번호와 일치합니다. 다시 입력해주십시오.\n > ");
                scanf("%s", t);
            }
            else{
                strcpy(a[i].pw, t);
                flag = 1;
            }
        }
    }//비밀번호 수정
    else if(choose_n3==3){
        printf(" 변경할 이름을 입력해주십시오.\n >> ");
        fgets(t,sizeof(t),stdin);
        t[strlen(t)-1] = '\0';
        while(flag == 0){
            if(strcmp(a[i].name, t) == 0){
                printf(" 기존의 이름과 일치합니다. 다시 입력해주십시오.\n > ");
                fgets(t,sizeof(t),stdin);
                t[strlen(t)-1] = '\0';
            }
            else{
                strcpy(a[i].name, t);
                flag = 1;
            }
        }
    }//이름 수정
    else if(choose_n3==4){
        printf(" 변경할 주소를 입력해주십시오.\n >>");
        fgets(t,sizeof(t),stdin);
        t[strlen(t)-1] = '\0'; 
        while(flag == 0){
            if(strcmp(a[i].address, t) == 0){
                printf(" 기존의 주소와 일치합니다. 다시 입력해주십시오.\n > ");
                fgets(t,sizeof(t),stdin);
                t[strlen(t)-1] = '\0';
            }
            else{
                strcpy(a[i].address, t);
                flag = 1;
            }
        }
    }//주소 수정
    else if(choose_n3==5){
        printf(" 수정할 금액을 입력해주십시오.\n >>");
        scanf("%d", &temp);
        while(flag == 0){
            if(a[i].balance == temp){
                printf(" 기존의 금액과 일치합니다. 다시 입력해주십시오.\n > ");
                scanf("%d", &temp);
            }
            else{
                a[i].balance = temp;
                flag = 1;
            }
        }
    }
    else if(choose_n3==6){
        printf(" 회원 탈퇴 여부를 입력해주십시오. ( 1 : 일반 회원 -1 : 탈퇴 회원.\n >>");
        scanf("%d", &temp);

        while(flag == 0){
            if(a[i].state == temp){
                printf(" 기존의 상태와 일치합니다. 다시 입력해주십시오.\n > ");
                scanf("%d", &temp);
            }
            else{
                a[i].state = temp;
                flag = 1;
            }
        }
    }//주소 수정 


    //    printf("%d,%s,%s,%s,%s,%d,%d\n",a[i].number, a[i].id, a[i].pw, a[i].name, a[i].address, a[i].balance, a[i].state);

    a_1 = a[i];

    return a_1;
}


customer customer_update(customer a_1, int choose_n3){
    char t[100];
    int flag = 0;
    char YN;
    if(choose_n3==1){
        if(pw_check(a_1) == 1){
            printf(" 확인되었습니다.\n");
            printf(" 변경할 비밀번호를 입력해주십시오.\n >> ");
            scanf("%s", t); 

            while(flag == 0){
                if(strcmp(a_1.pw, t) == 0){
                    printf(" 기존의 비밀번호와 일치합니다. 다시 입력해주십시오.\n > ");
                    scanf("%s", t);
                }
                else{
                    strcpy(a_1.pw, t);
                    flag = 1;
                }
            }
        }
    }//비밀번호 수정
    else if(choose_n3==2){
        printf(" 변경할 이름을 입력해주십시오.\n >> ");
        fgets(t,sizeof(t),stdin);
        t[strlen(t)-1] = '\0';
        while(flag == 0){
            if(strcmp(a_1.name, t) == 0){
                printf(" 기존의 이름과 일치합니다. 다시 입력해주십시오.\n > ");
                fgets(t,sizeof(t),stdin);
                t[strlen(t)-1] = '\0';
            }
            else{
                strcpy(a_1.name, t);
                flag = 1;
            }
        }
    }//이름 수정
    else if(choose_n3==3){
        printf(" 변경할 주소를 입력해주십시오.\n >>");
        getchar();
        fgets(t,sizeof(t),stdin);
        t[strlen(t)-1] = '\0'; 
        while(flag == 0){
            if(strcmp(a_1.address, t) == 0){
                printf(" 기존의 주소와 일치합니다. 다시 입력해주십시오.\n > ");
                fgets(t,sizeof(t),stdin);
                t[strlen(t)-1] = '\0';         
            }
            else{
                strcpy(a_1.address, t);
                flag = 1;
                printf(" 수정된주소 %s\n", a_1.address);
            }
        }
    }//주소 수정
    else if(choose_n3 == 4){
        printf("==================================\n");
        printf(" 본인 확인이 필요합니다.\n");
        if(pw_check(a_1)==1){

            printf(" !!! 정말 탈퇴하시겠습니까? (y/n) > ");
            getchar();
            scanf("%c",&YN);
            getchar();
            if(YN == 'y'){
                printf("************************************************************\n");
                printf("*  그동안 Day6 Would를 이용해 주셔서 감사합니다. 플챙유건! *\n");
                printf("************************************************************\n");
                printf(" \n Enter \n");
                getchar();
                a_1.balance = 0;
                a_1.state = -1;
            }
            else {
                printf(" 마이페이지로 돌아갑니다. \n"); 
                printf(" Enter\n");
                getchar();
            }
        }
    }

    return a_1;
}

void rewrite_history(history *c, history c_1){
    FILE* fpr;
    int n,i=0; 

    fpr = fopen("history.csv","r");
    n = CountReadfile(fpr);
    fclose(fpr);
    fpr = fopen("history.csv", "r");
    History_Readfile(fpr,c,n);
    fclose(fpr);

    FILE* fpw;
    fpw = fopen("history.csv","w");
    fprintf(fpw, "주문번호, 상품번호, 회원번호, 배송현황\n");    

    while(c[i].order_n != c_1.order_n){
        i++;
        if(i == n){
            //새로써야함
            c = (history*)realloc(c,sizeof(history)*(n+1));
            c[n] = c_1;
            for(i=0;i<n+1;i++){
                fprintf(fpw,"%d,%d,%d,%d\n",i+1,c[i].product_n,c[i].customer_n,c[i].state);
            }   
            break;
        }
    }
    i=0;

    while(c[i].order_n != c_1.order_n){
        i++;
    }

    if(i < n){
        c[i] = c_1;

        for(i=0;i<n;i++){
            fprintf(fpw,"%d,%d,%d,%d\n",i+1,c[i].product_n,c[i].customer_n,c[i].state);
        } 
    }    
    fclose(fpw);

}

void rewrite_customer(customer *a, customer a_1){
    FILE* fpr;
    int n,i=0; 

    fpr = fopen("customer.csv","r");
    n = CountReadfile(fpr);
    fclose(fpr);

    FILE* fpw;
    fpw = fopen("customer.csv","w");

    while(a[i].number != a_1.number){
        i++;   
    }
    a[i] = a_1;    
    fprintf(fpw, "회원번호, 아이디, 비밀번호, 이름, 주소, 잔액, 상태\n");    

    for(i=0;i<n;i++){
        fprintf(fpw,"%d,%s,%s,%s,%s,%d,%d\n",i+1,a[i].id,a[i].pw,a[i].name,a[i].address,a[i].balance, a[i].state);

    }   
    fclose(fpw);
}

void pro_new(product *b){ //새 상품 등록
    FILE* fpr;

    int n;
    char name[50],category[50];
    int price;

    fpr = fopen("product.csv","r");
    n = CountReadfile(fpr);
    fclose(fpr);

    b = (product*)malloc(sizeof(product)*n);

    fpr = fopen("product.csv","r");
    Product_Readfile(fpr, b, n);
    fclose(fpr);

    printf("=====등록할 상품의 정보를 입력하세요.=====\n");
    printf(" 상품명 : ");
    getchar();
    fgets(name,sizeof(name),stdin);
    name[strlen(name)-1] = '\0';
    
    printf(" 카테고리 : ");
    fgets(category,sizeof(category),stdin);
    category[strlen(category)-1] = '\0';
    
    printf(" 가격 : ");
    scanf("%d",&price);

    FILE* fpw = fopen("product.csv", "w");
    fprintf(fpw, "상품번호,상품명,카테고리,가격,상태\n");
    int i;

    if(n == 1){ 
        fprintf(fpw,"%d,%s,%s,%d,%d\n", 1,b[0].p_name,b[0].category,b[0].price,b[0].state);
        fprintf(fpw,"%d,%s,%s,%d,1\n", 2, name, category, price);
    }
    else{
        for(i = 0; i < n; i++){
printf("%d,%s,%s,%d,%d\n", i+1, b[i].p_name, b[i].category, b[i].price, b[i].state);
 
            fprintf(fpw,"%d,%s,%s,%d,%d\n", i+1, b[i].p_name, b[i].category, b[i].price, b[i].state);
        }
        n++;

        printf("%d, %s, %s, %d, 1\n", n, name, category, price);
        fprintf(fpw,"%d,%s,%s,%d,1\n", n, name, category, price);
    }
    fclose(fpw);
}

void pro_dlt(product *b){ // 상품 삭제
    char name[50];
    int number;
    FILE* fpr;
    int n;

    fpr = fopen("product.csv","r");
    n = CountReadfile(fpr);
    fclose(fpr);

    b = (product*)malloc(sizeof(product)*n);

    fpr = fopen("product.csv","r");
    Product_Readfile(fpr, b, n);
    fclose(fpr);

    printf("==삭제할 상품의 정보를 입력하세요.==\n");
    printf("1. 상품번호\n2. 이름\n0. 이전 화면으로 돌아가기\n선택 > ");
    int option, i;
    scanf("%d",&option);

    switch(option){
        case 1: printf(" 상품번호 : ");
                scanf("%d", &number);
                for(i = 0; i < n; i++){
                    if(b[i].number == number){
                        b[i].state = -1;
                    }
                }
                break;
        case 2: printf(" 상품명 : ");
                getchar();
                fgets(name,sizeof(name),stdin);
                name[strlen(name)-1] = '\0';
                for(i = 0; i < n; i++){
                    if(strcmp(b[i].p_name, name) == 0){
                        b[i].state = -1;
                    }
                }
                break;
        default: break;
    }

    FILE* fpw = fopen("product.csv", "w");
    fprintf(fpw, "상품번호,상품명,카테고리,가격,상태\n");
    if(n == 1){
        fprintf(fpw,"%d,%s,%s,%d,%d\n", 1,b[0].p_name,b[0].category,b[0].price,b[0].state);
    }
    else{
        for(i = 0; i < n; i++){
            fprintf(fpw,"%d,%s,%s,%d,%d\n", i+1, b[i].p_name, b[i].category, b[i].price, b[i].state);
        }
    }
    fclose(fpw);

}

void pro_rew(product *b){ // 상품 수정
    char name[50];
    int number;
    FILE* fpr;
    int n;
    int choose_n;
    int t;
    char temp[50];

    fpr = fopen("product.csv","r");
    n = CountReadfile(fpr);
    fclose(fpr);

    b = (product*)malloc(sizeof(product)*n);

    fpr = fopen("product.csv","r");
    Product_Readfile(fpr, b, n);
    fclose(fpr);

    printf(" 수정할 상품의 정보를 입력하세요.\n");
    printf("1. 상품번호 \n2. 이름\n0. 이전 화면으로 돌아가기\n선택 > ");
    int option, i;
    scanf("%d",&option);

    switch(option){
        case 1: printf(" 상품번호 : ");
                scanf("%d", &number);
                for(i = 0; i < n; i++){
                    if(b[i].number == number){
                        printf("=====해당 상품의 어떤 정보를 수정하시겠습니까?=====\n");
                        printf(" 1. 상품명\n 2. 카테고리\n 3. 가격\n 4. 판매가능여부\n 선택 >");
                        scanf("%d", &choose_n);

                        if(choose_n == 1){
                            printf(" 상품명 : %s\n", b[i].p_name);
                            printf(" 수정할 내용을 입력해주십시오\n >> ");

                            getchar();
                            fgets(temp,sizeof(temp),stdin);
                            temp[strlen(temp)-1] = '\0';
                            strcpy(b[i].p_name, temp);
                            printf(" 수정이 완료되었습니다. Enter을 입력하면 메인화면으로 돌아갑니다.\n");
                            getchar();
                            getchar();
                        }
                        else if(choose_n == 2){
                            printf(" 카테고리 : %s\n", b[i].p_name);
                            printf(" 수정할 내용을 입력해주십시오\n >> ");
                            getchar();
                            fgets(temp,sizeof(temp),stdin);
                            temp[strlen(temp)-1] = '\0';

                            strcpy(b[i].category, temp);
                            printf(" 수정이 완료되었습니다. Enter을 입력하면 메인화면으로 돌아갑니다.\n");
                            getchar();
                            getchar();
                        }
                        else if(choose_n == 3){
                            printf(" 가격 : %s\n", b[i].p_name);
                            printf(" 수정할 내용을 입력해주십시오\n >> ");
                            scanf("%d", &t);

                            b[i].price = t;
                            printf(" 수정이 완료되었습니다. Enter을 입력하면 메인화면으로 돌아갑니다.\n");
                            getchar();
                            getchar();
                        }
                        else if(choose_n == 4){
                            if(b[i].state == 1) printf(" 판매가능여부 : 판매중\n");
                            printf(" 수정할 내용을 입력해주십시오\n >> ");
                            scanf("%d", &t);
                            b[i].state = t;
                            printf(" 수정이 완료되었습니다. 메인화면으로 돌아갑니다.\n Enter\n");
                            getchar();
                        }
                        break;
                    }
                }
                break;
        case 2: printf(" 상품명 : ");

                getchar();
                fgets(name,sizeof(name),stdin);
                name[strlen(name)-1] = '\0';

                for(i = 0; i < n; i++){
                    if(strcmp(b[i].p_name, name) == 0){
                        printf("=====해당 상품의 어떤 정보를 수정하시겠습니까?=====\n");
                        printf(" 1. 상품명\n 2. 카테고리\n 3. 가격\n 4. 판매가능여부\n 선택 >");
                        scanf("%d", &choose_n);

                        if(choose_n == 1){
                            printf(" 상품명 : %s\n", b[i].p_name);
                            printf(" 수정할 내용을 입력해주십시오\n >> ");
                            getchar();

                            fgets(temp,sizeof(temp),stdin);
                            temp[strlen(temp)-1] = '\0';
                            strcpy(b[i].p_name, temp);

                            printf(" 수정이 완료되었습니다. Enter을 입력하면 메인화면으로 돌아갑니다.\n");
                            getchar();
                        }
                        else if(choose_n == 2){
                            printf(" 카테고리 : %s\n", b[i].p_name);
                            printf(" 수정할 내용을 입력해주십시오\n >> ");
                            getchar();    
                            fgets(temp,sizeof(temp),stdin);
                            temp[strlen(temp)-1] = '\0';
                            strcpy(b[i].category, temp);
                            printf(" 수정이 완료되었습니다. Enter을 입력하면 메인화면으로 돌아갑니다.\n");
                            getchar();
                        }
                        else if(choose_n == 3){
                            printf(" 가격 : %s\n", b[i].p_name);
                            printf(" 수정할 내용을 입력해주십시오\n >> ");
                            scanf("%d", &t);
                            b[i].price = t;
                            printf(" 수정이 완료되었습니다. Enter을 입력하면 메인화면으로 돌아갑니다.\n");
                            getchar();
                        }
                        else if(choose_n == 4){
                            if(b[i].state == 1) printf(" 판매가능여부 : 판매중\n");
                            printf(" 수정할 내용을 입력해주십시오\n >> ");
                            scanf("%d", &t);
                            b[i].state = t;
                            printf(" 수정이 완료되었습니다. Enter을 입력하면 메인화면으로 돌아갑니다.\n");
                            getchar();
                        }
                        break;
                    }                                         
                }
                break;
        default: break;
    }

    FILE* fpw = fopen("product.csv", "w");
    fprintf(fpw, "상품번호,상품명,카테고리,가격,상태\n");
    if(n == 1){
        fprintf(fpw,"%d,%s,%s,%d,%d\n", 1,b[0].p_name,b[0].category,b[0].price,b[0].state);
    }
    else{
        for(i = 0; i < n; i++){
            fprintf(fpw,"%d,%s,%s,%d,%d\n", i+1, b[i].p_name, b[i].category, b[i].price, b[i].state);
        }
    }
    fclose(fpw);

}

void Banking(customer *a_1, customer *a){
    int flag3 = 0;
    int choose_n3, coin;

    while(flag3 == 0){
        printf("==================================\n");
        printf(" %s님의 가상 계좌 잔액 : %d원\n", a_1->name, a_1->balance);
        printf(" 1. 입금하기 \n 2. 출금하기 \n 3. 이전 페이지로 돌아가기 \n 선택 > ");
        scanf("%d", &choose_n3);

        if(choose_n3 == 1){
            printf("==================================\n");
            printf(" 입금할 금액을 입력하십시오 > ");
            scanf("%d",&coin);
            printf("==================================\n");
            a_1->balance = a_1->balance + coin;
            printf(" 입금이 완료되었습니다.\n");
            rewrite_customer(a, *a_1);
        }
        else if(choose_n3 == 2){
            while(1){
                printf("==================================\n");
                printf(" 출금할 금액을 입력하십시오 > ");       
                scanf("%d", &coin);
                printf("==================================\n");

                if(a_1->balance - coin < 0){
                    printf(" 최대 %d원 출금 가능합니다.\n", a_1->balance);
                }
                else{
                    a_1->balance = a_1->balance - coin;
                    printf(" 출금이 완료되었습니다.\n");
                    rewrite_customer(a, *a_1);
                    break;
                }
            }
        }   
        else if(choose_n3 == 3){    
            flag3 = 1;
        }
    }                       
}

int Product_Search(int loginFlag, product* b_1){
    int choose_n;
    int n,i,pn,n2,cnt=0;
    int flag1 = 0;
    int flag2 = 0;
    char sname[50];
    char scategory[50];
    int spnum;
    FILE* fpr;
    product *b;

    fpr = fopen("product.csv","r");
    n = CountReadfile(fpr);
    fclose(fpr);

    b = (product*)malloc(sizeof(product)*n);

    fpr = fopen("product.csv","r");
    Product_Readfile(fpr, b, n);
    fclose(fpr);

    while(flag1 == 0){
        printf("=====상품 검색하기=====\n");
        printf(" 1. 상품번호로 검색하기\n");
        printf(" 2. 상품이름으로 검색하기\n");
        printf(" 3. 카테고리로 검색하기\n");
        printf(" 0. 이전 화면으로 돌아가기\n");

        printf(" 선택 > ");
        scanf("%d",&choose_n);

        if(choose_n == 1){

            printf(" 검색어 > ");
            scanf("%d",&pn);
            printf("========================\n");
            for(i=0;i<n;i++){
                if(pn == b[i].number){
                    printf(" 상품번호 : %d번\n", b[i].number);
                    printf(" 상품이름 : %s\n", b[i].p_name);
                    printf(" 카테고리 : %s\n", b[i].category);
                    printf(" 가격 : %d원\n", b[i].price);
                    printf("========================\n");
                    cnt++;
                }
            }
            if(cnt == 0){
                printf(" '%d' 에 대한 Day6 Would 내 검색결과가 없습니다.\n", pn);
            }
        }
        else if(choose_n == 2 ){
            printf(" 검색어 > ");

            getchar();        
            fgets(sname,sizeof(sname),stdin);   
            sname[strlen(sname)-1] = '\0';

            printf("========================\n");
            for(i=0;i<n;i++){ 
                if(strcmp(sname,b[i].p_name) == 0){
                    printf(" 상품번호 : %d번\n", b[i].number);
                    printf(" 상품이름 : %s\n", b[i].p_name);
                    printf(" 카테고리 : %s\n", b[i].category);
                    printf(" 가격 : %d원\n", b[i].price);
                    printf(" ========================\n");
                    cnt++; 
                }
            }
            printf("%d\n",cnt);
            if(cnt == 0){
                printf(" '%s' 에 대한 Day6 Would 내 검색결과가 없습니다.\n", sname);  
            }
        }
        else if(choose_n == 3){

            printf(" 검색어 > ");

            getchar();
            fgets(scategory,sizeof(scategory),stdin);
            scategory[strlen(scategory)-1] = '\0';

            printf("========================\n");
            for(i=0;i<n;i++){
                if(strcmp(scategory,b[i].category) == 0){
                    printf(" 상품번호 : %d번\n", b[i].number);
                    printf(" 상품이름 : %s\n", b[i].p_name);
                    printf(" 카테고리 : %s\n", b[i].category);
                    printf(" 가격 : %d원\n", b[i].price);
                    printf("========================\n");
                    cnt++;           
                }
            }
            if(cnt == 0){
                printf(" '%s' 에 대한 Day6 Would 내 검색결과가 없습니다.\n",scategory);
            }
        }
        else if(choose_n == 0){
            return 0;
        }

        if(cnt != 0){
            while(flag2 == 0){
                printf(" 원하시는 상품의 상품번호를 입력해주십시오.\n (이전화면으로 돌아가려면 0입력)\n 선택 > ");
                scanf("%d",&pn);
                if(pn == 0) break;
                pn--;
                if(b[pn].state != 1){
                    printf(" **해당 상품은 현재 판매중이 아닙니다.**\n");
                }
                else{
                    printf(" 1. 구매하기\n 0. 이전화면으로 돌아가기\n");
                    printf(" 선택 > ");
                    scanf("%d",&n2);
                    if(n2 == 0) continue;
                    else{
                        if(loginFlag == 0){
                            printf(" **이 상품을 구매하기 위해선 로그인을 해야합니다.**\n");
                            printf(" 1. 다른 상품 검색하기.\n 0.메인화면으로 돌아가기\n 선택 > ");
                            scanf("%d",&n2);

                            if(n2 == 1){
                                continue;
                            }
                            else if(n2 == 0){
                                flag1 = 1;
                                return -1;//로그인안하고 메인으로 돌아가는거
                            }
                        }
                        else if(loginFlag == 1){
                            *b_1 = b[pn];
                            pn++;
                            return pn;
                            //상품구매함수
                        }

                    }
                }
            }
        }
    }
}

void Buying(customer a_1, int p_number, history *c){
    int n;
    FILE* fpr;

    fpr = fopen("history.csv","r");
    n = CountReadfile(fpr);
    fclose(fpr);

    c = (history*)malloc(sizeof(history)*n);

    history c_1;

    c_1.order_n = n+1;
    c_1.product_n = p_number;
    c_1.customer_n = a_1.number;
    c_1.state = 0;   
    rewrite_history(c, c_1);
    printf(" 상품 구매가 완료되었습니다.\n");

}
void show_del(customer *a, customer a_1, product *b, history *c){
    int i,pn,n;
    FILE* fpr;

    fpr = fopen("product.csv","r");
    n = CountReadfile(fpr);
    fclose(fpr);
    b = (product*)malloc(sizeof(product)*n);
    fpr = fopen("product.csv","r");
    Product_Readfile(fpr, b, n);
    fclose(fpr);

    fpr = fopen("history.csv","r");
    n = CountReadfile(fpr);
    fclose(fpr);
    c = (history*)malloc(sizeof(product)*n);
    fpr = fopen("history.csv","r");
    History_Readfile(fpr, c, n);
    fclose(fpr);

    printf("=====%s님의 배송중인 상품 목록=====\n", a_1.name);

    for(i=0;i<n;i++){
        if(a_1.number == c[i].customer_n && c[i].state == 1){
            pn = c[i].product_n;
            pn--;
            printf(" 상품번호 : %d\n", b[pn].number);
            printf(" 상품이름 : %s\n", b[pn].p_name);
            printf(" 카테고리 : %s\n", b[pn].category);
            printf("===================================\n");        
        }
    }
    printf(" 메인화면으로 돌아갑니다.\n Enter\n");
    getchar();
    getchar();
}

void cus_buylist(customer a_1, product *b, history *c){
    FILE* fpr;
    int choose_n1;
    int i,j ,temp,pn,hn;
    int cnt = 0;
    char category[50];
    
    fpr = fopen("product.csv","r");
    pn = CountReadfile(fpr);
    fclose(fpr);
    b = (product*)malloc(sizeof(product)*pn);
    fpr = fopen("product.csv","r");
    Product_Readfile(fpr, b, pn);
    fclose(fpr);

    fpr = fopen("history.csv","r");
    hn = CountReadfile(fpr);
    fclose(fpr);
    c = (history*)malloc(sizeof(product)*hn);
    fpr = fopen("history.csv","r");
    History_Readfile(fpr, c, hn);
    fclose(fpr);

    printf("=====구매 내역 조회 페이지=====\n");
    printf("1. 전체 내역 조회\n2. 카테고리로 검색\n0. 이전 화면으로 돌아가기\n 선택 > ");   
    
//history함수 n값,product 내용

    scanf("%d",&choose_n1);
    switch(choose_n1){
        case 1: printf("======전체 구매 내역======\n");
                printf("   상품명 / 카테고리 / 가격 \n");
                for(i = 0; i < hn; i++){
                    if(a_1.number == c[i].customer_n){    
                        for(j=0;j<pn;j++){
                            if(c[i].product_n == b[j].number){
                                printf(" %d. %s / %s / %d원\n", cnt+1, b[j].p_name, b[j].category, b[j].price); 
                            cnt++;
                            }
                        }
                    }   
                }
                printf("=========================\n");

                break;
        case 2: printf(" 검색어 > ");
                
                getchar();
                fgets(category,sizeof(category),stdin);
                category[strlen(category)-1] = '\0';
                
                printf("======='%s'에서의 구매 내역=======\n", category);
                for(i = 0;i<hn;i++){
                    if(a_1.number == c[i].customer_n){
                        for(j=0;j<pn;j++){
                            if(c[i].product_n == b[j].number && strcmp(category, b[j].category) == 0){
    
                                printf(" %d. %s / %s / %d원\n", cnt+1, b[j].p_name, b[j].category, b[j].price); 
                                cnt++;
                            }
                        }
                    }
                }
                printf("===================================\n");
               break;
        default : return;
    }
    if(cnt == 0) printf(" 구매 내역이 없습니다. \n");

    printf(" 이전 화면으로 돌아갑니다.\n Enter\n");
    getchar();
    getchar();
}


void del(history* c){
    FILE* fpr;
    int n, number;

    fpr = fopen("history.csv","r");
    n = CountReadfile(fpr);
    fclose(fpr);

    c = (history*)malloc(sizeof(history)*n);

    fpr = fopen("history.csv","r");
    History_Readfile(fpr, c, n);
    fclose(fpr);

    printf("=====배송내역 수정 페이지=====\n");
    printf(" 1. 주문번호를 입력하여 배송 정보 수정\n 0. 이전 화면으로 돌아가기\n 선택 > ");
    int option, i;
    scanf("%d",&option);

    switch(option){
        case 1: printf(" 주문번호 입력 >> ");
                scanf("%d", &number);
                for(i = 0; i < n; i++){
                    if(c[i].order_n == number){
                        printf(" 주문번호 %d번의 배송 현황 :", c[i].order_n);
                        if(c[i].state == 0) printf(" 배송준비\n");
                        else if(c[i].state == 1) printf(" 배송중\n");
                        else if(c[i].state == 2) printf(" 배송완료\n");
                        
                        printf("( 0 : 배송준비, 1 : 배송중, 2 : 배송완료 )\n >> ");
                        scanf("%d", &c[i].state);
 
                        printf("배송내역 수정이 완료되었습니다. Enter입력시 메인화면으로 돌아갑니다.\n");
                        getchar();
                   }
                }
                break;
        default: break;
    }

    FILE* fpw = fopen("history.csv", "w");
    fprintf(fpw, "주문번호,상품번호,회원번호,배송현황\n");
    if(n == 1){
        fprintf(fpw,"%d,%d,%d,%d\n", 1,c[0].product_n,c[0].customer_n,c[0].state);
    }
    else{
        for(i = 0; i < n; i++){
            fprintf(fpw,"%d,%d,%d,%d\n",c[i].order_n,c[i].product_n,c[i].customer_n,c[i].state);
        }
    }
    fclose(fpw);

}

int cnt(history* c, int hn, int n){
    int i,j=0;
    for(i=0;i<hn;i++){
        if(c[i].product_n == n) j++;
    }

    return j;
}

void show_sell(product *b, history *c){
    FILE* fpr;
    int pn,hn;

    fpr = fopen("product.csv","r");
    pn = CountReadfile(fpr);
    fclose(fpr);
    b = (product*)malloc(sizeof(product)*pn);
    fpr = fopen("product.csv","r");
    Product_Readfile(fpr, b, pn);
    fclose(fpr);

    fpr = fopen("history.csv","r");
    hn = CountReadfile(fpr);
    fclose(fpr);
    c = (history*)malloc(sizeof(product)*hn);
    fpr = fopen("history.csv","r");
    History_Readfile(fpr, c, hn);
    fclose(fpr);


    int i,j,choose_n1,choose_n2;
    int pnumber;
    char pname[50];

    printf("=====판매 내역 및 통계 조회 페이지=====\n");
    printf(" 1. 전체 상품별 판매량 조회\n 2. 개별 상품 판매량 조회\n 0. 이전으로 돌아가기\n 선택 > ");
    scanf("%d", &choose_n1);

    if(choose_n1 == 1){
        printf("===================================\n");
        for(i=0;i<pn;i++){
        printf(" ( %d번 / %s / %s ) -> %d개 판매\n", b[i].number, b[i].p_name, b[i].category, cnt(c, hn, b[i].number));
        }
         printf("===================================\n");
    }
    else if(choose_n1 == 2){
        printf("==================================\n");
        printf(" 1. 상품번호 검색\n 2. 상품명 검색\n 선택 > ");
        scanf("%d", &choose_n2);
        if(choose_n2 == 1){
            printf(" 상품번호 입력 >> ");
            scanf("%d",&pnumber);
            for(i=0;i<pn;i++){
                if(pnumber == b[i].number){
                    printf(" ( %d번 / %s / %s ) -> %d개 판매\n", b[i].number, b[i].p_name,b[i].category, cnt(c,hn,b[i].number));
                }
            }
        }
        else if(choose_n2 == 2){
            printf(" 상품명 입력 >> ");
            getchar();
            fgets(pname,sizeof(pname),stdin);
            pname[strlen(pname)-1] = '\0';
            
            for(i=0;i<pn;i++){
                if(strcmp(pname,b[i].p_name) == 0){
                printf(" ( %d번 / %s / %s ) -> %d개 판매\n", b[i].number, b[i].p_name,b[i].category, cnt(c,hn,b[i].number));

                }

            }
        }

    }

}


void show_product(product *b, history *c){
    FILE* fpr;
    
    int choose_n1,choose_n2,pn,hn,k,i,j,t1, t2;
    int *t;
    product tt;

    fpr = fopen("product.csv","r");
    pn = CountReadfile(fpr);
    fclose(fpr);
    b = (product*)malloc(sizeof(product)*pn);
    fpr = fopen("product.csv","r");
    Product_Readfile(fpr, b, pn);
    fclose(fpr);

    fpr = fopen("history.csv","r");
    hn = CountReadfile(fpr);
    fclose(fpr);
    c = (history*)malloc(sizeof(product)*hn);
    fpr = fopen("history.csv","r");
    History_Readfile(fpr, c, hn);
    fclose(fpr);

    t = (int*)malloc(sizeof(t)*pn);

    while(1){
        printf("=====상품 조회 및 통계 페이지=====\n");
        printf(" 1. 전체 상품 조회\n 2. 상품 검색\n 3. 상품 통계\n 0. 이전화면으로 돌아가가기\n 선택 > ");
        scanf("%d",&choose_n1);
        if(choose_n1 == 1){
                    printf(" 상품번호, 상품명, 카테고리, 가격, 상태\n");
                    for(i=0;i<pn;i++){
                        printf(" %d번 / %s / %s / %d원 / %d\n", b[i].number, b[i].p_name, b[i].category, b[i].price, b[i].state);
                    }
        }
        else if(choose_n1 == 2){

        }
        else if(choose_n1 == 0){
            break;
        }
    }
}

