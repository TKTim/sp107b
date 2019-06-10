#include <assert.h>
#include "compiler.h"

int E();
void STMT();
void IF();
void Switch();
void BLOCK();
void explain();

int tempIdx = 0, labelIdx = 0;

#define nextTemp() (tempIdx++)
#define nextLabel() (labelIdx++)
#define emit printf

int isNext(char *set) {
  char eset[SMAX], etoken[SMAX];
  sprintf(eset, " %s ", set);
  sprintf(etoken, " %s ", tokens[tokenIdx]);
  return (tokenIdx < tokenTop && strstr(eset, etoken) != NULL);
}

int isEnd() {
  return tokenIdx >= tokenTop;
}

char *next() {
  // printf("token[%d]=%s\n", tokenIdx, tokens[tokenIdx]);
  return tokens[tokenIdx++];
}

char *skip(char *set) {
  if (isNext(set)) {
    return next();
  } else {
    printf("skip(%s) got %s fail!\n", set, next());
    assert(0);
  }
}

// F = (E) | Number | Id
int F() {
  int f;
  if (isNext("(")) { // '(' E ')'
    next(); // (
    f = E();
    next(); // )
  } else { // Number | Id
    f = nextTemp();
    char *item = next();
    emit("t%d = %s\n", f, item);
  }
  return f;
}

// E = F (op E)*
int E() {
  int i1 = F();
  while (isNext("+ - * / & | ! < > =")) {
    char *op = next();
    int i2 = E();
    int i = nextTemp();
    emit("t%d = t%d %s t%d\n", i, i1, op, i2);
    i1 = i;
  }
  return i1;
}



// ASSIGN = id '=' E;
void ASSIGN() {
  char *id = next();
  skip("=");
  int e = E();
  skip(";");
  emit("%s = t%d\n", id, e);
}

// while (E) STMT
void WHILE() {
  int whileBegin = nextLabel();
  int whileEnd = nextLabel();
  emit("(L%d)\n", whileBegin);
  skip("while");
  skip("(");
  int e = E();
  emit("if not T%d goto L%d\n", e, whileEnd);
  skip(")");
  STMT();
  emit("goto L%d\n", whileBegin);
  emit("(L%d)\n", whileEnd);
}
 
// 註解 
void explain(){ 
  skip("/");
  skip("*");
  while (!isNext("*"))   {  tokenIdx++; }
  skip("*");
  skip("/");
  STMT();
}

void IF() {
   int elseBegin = nextLabel();
   int endifLabel = nextLabel();
   skip("if");
   skip("(");
   int e = E();
   skip(")");
   emit("ifnot t%d goto L%d\n", e, elseBegin);
   STMT();
   emit("goto L%d\n", endifLabel);
   if (isNext("else")) {
     emit("(L%d)\n", elseBegin);
     skip("else");
     STMT();
   } 
   emit("(L%d)\n", endifLabel);
 }

 //switch
 void Switch(){
    int nextone = nextLabel();
    skip("switch");
    skip("(");
    int e = E(); //讀取switch 的 條件式
    skip(")");
    skip("{");
    // 計算 總共的case以助於 使用(L?) 來增快執行速度
    int temp = tokenIdx;
    int count = 1;
    while (!isNext("default"))
    {
      if(isNext("case")) count++;
      tokenIdx++;
    }
    int endlabel = nextone + count; // (L endlabel)
    tokenIdx = temp;
    // 再將取出來使用的tokenIdx 改回原值 
    while (isNext("case")) {
      skip("case");
      emit("(L%d)\n", nextone);
      int cc = E();
      int i = nextTemp();
      emit("t%d = t%d != t%d\n", i, e, cc);
      nextone = nextLabel();
      emit("if t%d goto L%d\n", i, nextone);
      skip(":");
      STMT();
      skip("break");
      skip(";");
      emit("goto L%d\n", endlabel);
      }
    if (isNext("default")) {
      skip("default");
      skip(":");
      emit("(L%d)\n", nextone);
      STMT();
      skip("}");
    }
    emit("(L%d)\n", endlabel);
    //輸出最後的 (L?)
 }


void STMT() {
  if (isNext("while"))
    return WHILE();
  else if (isNext("if"))  
     IF();               
  else if(isNext("switch")) // switch
     Switch(); 
  else if(isNext("/"))    // 註解
     explain();
  else if (isNext("{"))
    BLOCK();
  else
    ASSIGN();
}


void STMTS() {
  while (!isEnd() && !isNext("}")) {
    STMT();
  }
}

// { STMT* }
void BLOCK() {
  skip("{");
  STMTS();
  skip("}");
}

void PROG() {
  STMTS();
}

void parse() {
  printf("============ parse =============\n");
  tokenIdx = 0;
  PROG();
}