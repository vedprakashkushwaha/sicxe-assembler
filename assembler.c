#define START "START"
#define END "END"
#define BYTE "BYTE"
#define WORD "WORD"
#define RESB "RESB"
#define RESW "RESW"
#define BASE "BASE"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
struct instruction{
	int loc;
	char label[16];
	char opcode[16];
	char operand[16];
	int object_code;
};
struct symTab{
	int loc;
	char label[16];
};
struct opcodeTab{
	int num;
	char opcode[16];
};
struct registers{
	char name[2];
	int num;
};

void display(struct instruction *inst, int line);
void object_program(struct instruction *inst, int line);
char *truncate(char ch[16]);
void generate_location(struct instruction *inst,int line);
void generate_object(struct instruction *inst,int line);
int code_len(int x);

int main() 
{
	FILE *fp;
	char ch[64],temp[10],label[10],opcode[10],operand[10];
	int flag=0,count,state=1,i,j,t=0;
	static struct  instruction inst[128];
	fp=fopen("sicxe.txt","r");
	while(fgets(ch,64,fp)!=NULL) 
	{
		count=strlen(ch);
		strcpy(opcode,"\0");
		strcpy(label,"\0");
		strcpy(operand,"\0");
		if(ch[0]=='\n') continue;	
		for(i=0,j=0;i<count;i++)
		{
			if(ch[i]=='\t' || ch[i]=='\n' || ch[i]==' ')
			{
				temp[j]='\0';
				j=0;
				if(strlen(temp)>0)
				{
					if(state==1)
					{
						strcpy(label,temp);
						state=2;
					}
					else if(state==2)
					{
						strcpy(opcode,temp);
						state=3;
					}
					else if(state==3)
					{
						strcpy(operand,temp);
						state=1;
					}
				}
			}
			else
			{
				if(ch[i]!='.')
				{
					temp[j]=ch[i];
					j++;
				}
			}
		}
		if(state==2)
		{
			strcpy(opcode,label);
			strcpy(label,"\0");
			strcpy(operand,"\0"); 
		}		
		if(state==3)
		{
			strcpy(operand,opcode);
			strcpy(opcode,label);
			strcpy(label,"\0"); 
		}			
		state=1;
		if(strlen(opcode)>0)
		{	
			strcpy(inst[t].label,label);
			strcpy(inst[t].opcode,opcode);
			strcpy(inst[t].operand,operand);
			inst[t].object_code=0;
			//strcpy(inst[t].object_code,"");
			t++;
		}
	}
	generate_location(inst,t);
	return(0);           
} 

void generate_location(struct instruction *instr,int line)
{
	struct instruction inst;
	int i,sym_count=0;
	FILE *temp,*symtab;
	int loc=0x0;
	char program_name[16];
	symtab=fopen("symtab.txt","w");
	for(i=0;i<line;i++)
	{
		inst=*(instr+i);
		if(strcmp(inst.opcode,START)==0)
		{
			strcpy(program_name,inst.label);
			temp=fopen("temp.txt","w");
			fprintf(temp,"%s",inst.operand);
			fclose(temp);
			temp=fopen("temp.txt","r");
			fscanf(temp,"%x",&loc);
			fclose(temp);
			inst.loc=loc;
			*(instr+i)=inst;			
			continue;


		}
		inst.loc=loc;
		if(strlen(inst.label)>0)
		{
			fprintf(symtab,"%x\t%s\n",loc,inst.label);
		}
		*(instr+i)=inst;
		if(strcmp(inst.opcode,BASE)==0)
		{

		}
		else if(strcmp(inst.opcode,"CLEAR")==0 || strcmp(inst.opcode,"COMPR")==0 || strcmp(inst.opcode,"TIXR")==0)
		{
			loc=loc+2;
		}
		else if(inst.opcode[0]=='+')
		{
			loc=loc+4;
		}
		else if(strcmp(inst.opcode,BYTE)==0)
		{
			if(inst.operand[0]=='x' || inst.operand[0]=='X')
			{
				if(strlen(inst.operand)%2!=0)
				{
					loc=loc+((strlen(inst.operand)-3)/2);
				}
				else
				{
					loc=loc+((strlen(inst.operand)-3)/2)+1;
				}
			}
			else
			{

				loc=loc+(strlen(inst.operand)-3);
			}
		}
		else if(strcmp(inst.opcode,RESW)==0)
		{
			loc=loc+atoi(inst.operand)*3;
		}
		else if(strcmp(inst.opcode,RESB)==0)
		{
			loc=loc+atoi(inst.operand);
		}
		else
		{
			loc=loc+3;
		}

	}
	fclose(symtab);
	generate_object(instr,line);
}

void generate_object(struct instruction *instr,int line)
{
	int i,sym_count=0,opcode_count=0,j,flag=0,flag1=0,k,temp1;	
	struct instruction inst,inst1;
	struct symTab symList[32];
	struct opcodeTab opcodeList[64];
	int object_code=0x0,comptemp=0;
	FILE *symtab,*opcodetab, *tempFp;
	char temp_operand[16],blank[2],object_codex[8],base[16];
	struct registers reg[10];

	strcpy(reg[0].name,"A");
	reg[0].num=0;
	strcpy(reg[1].name,"L");
	reg[1].num=2;
	strcpy(reg[2].name,"PC");
	reg[2].num=8;
	strcpy(reg[3].name,"SW");
	reg[3].num=9;
	strcpy(reg[4].name,"B");
	reg[4].num=3;
	strcpy(reg[5].name,"S");
	reg[5].num=4;
	strcpy(reg[6].name,"T");
	reg[6].num=5;
	strcpy(reg[7].name,"F");
	reg[7].num=6;
	strcpy(reg[8].name,"X");
	reg[8].num=1;

	blank[0]='\0';
	symtab=fopen("symtab.txt","r");
	opcodetab=fopen("opcode_table.txt","r");

	while(fscanf(symtab,"%x\t%s",&symList[sym_count].loc,symList[sym_count].label)!=EOF)
	{
		sym_count++;
	}

	while(fscanf(opcodetab,"%s\t%x",opcodeList[opcode_count].opcode,&opcodeList[opcode_count].num)!=EOF)
	{
		opcode_count++;
	}

	inst=*(instr+0);
	//printf("%04x\t%-10s%-10s%-10s\t%s\n",inst.loc,inst.label,inst.opcode,inst.operand,blank);
	for(i=1;i<line;i++)
	{
		inst=*(instr+i);
		strcpy(temp_operand,inst.operand);
		if(strcmp(inst.opcode,"BASE")==0)
		{
			strcpy(base,inst.operand);
			continue;
		}
		if(strcmp(inst.opcode,"CLEAR")==0 || strcmp(inst.opcode,"COMPR")==0 || strcmp(inst.opcode,"TIXR")==0)
		{
			for(j=0;j<opcode_count;j++)
			{
				if(strcmp(inst.opcode,opcodeList[j].opcode)==0)
				{
					object_code=opcodeList[j].num;
					break;
				}
			}
			if(strlen(inst.operand)==1)
			{
				for(j=0;j<9;j++)
				{
					if(strcmp(reg[j].name,inst.operand)==0)
					{
						object_code=object_code<<4;
						object_code|=reg[j].num;
						object_code=object_code<<4;
						break;
					}	
				}
			}
			else
			{
				for(j=0;j<9;j++)
				{
					if(reg[j].name[0]==inst.operand[0] && reg[j].name[1]=='\0')
					{
						object_code=object_code<<4;
						object_code|=reg[j].num;
						break;
					}	
				}
				for(j=0;j<9;j++)
				{
					if(reg[j].name[0]==inst.operand[2] && reg[j].name[1]=='\0')
					{
						object_code=object_code<<4;
						object_code|=reg[j].num;
						break;
					}	
				}
			}
			inst.object_code=object_code;
			*(instr+i)=inst;
			//printf("%04x\t%-10s%-10s%-10s\t%04x\n",inst.loc,inst.label,inst.opcode,inst.operand,inst.object_code);
			continue;
		}


		if(inst.operand[0]=='#'||inst.operand[0]=='@')
		{
			strcpy(temp_operand,truncate(temp_operand));	
		}

		if(strcmp(inst.opcode,BYTE)==0)
		{
			object_code=0;
			if(inst.operand[0]=='X' || inst.operand[0]=='x')
			{
				for(j=0;j+3<strlen(inst.operand);j++)
				{

					object_codex[j]=inst.operand[j+2];
				}   
				object_codex[j]='\0';

				tempFp = fopen("temp.txt","w");
				fprintf(tempFp,"%s",object_codex);
				fclose(tempFp);

				tempFp = fopen("temp.txt","r");
				fscanf(tempFp,"%x",&object_code);
				fclose(tempFp);

				inst.object_code=object_code;
				*(instr+i)=inst;
				//printf("%04x\t%-10s%-10s%-10s\t%02x\n",inst.loc,inst.label,inst.opcode,inst.operand,inst.object_code);
				continue;
			}
			else
			{
				for(j=0;j+3<strlen(inst.operand);j++)
				{
					temp1=0x0;
					temp1=temp1+(int)inst.operand[j+2];
					object_code=object_code* 0x100 + temp1;
				}   
			}
		}
		else if(strcmp(inst.opcode,RESB)==0 || strcmp(inst.opcode,RESW)==0 || strcmp(inst.opcode,BASE)==0)
		{
			object_code=0;
		}
		else
		{
			flag=0;
			for(j=0;j<opcode_count;j++)
			{
				if(strcmp(inst.opcode,opcodeList[j].opcode)==0)
				{
					object_code=opcodeList[j].num;
					flag=1;
					break;
				}
			}
			if(flag)
			{
				if(inst.operand[0]=='#')
				{
					//object_code &=0xfd;
					object_code |=1;
					object_code=object_code<<4;
				}
				else if(inst.operand[0]=='@')
				{
					object_code |=2;
					object_code=object_code<<4;
				}
				else
				{
					object_code |=3;
					object_code=object_code<<4;
				}

				flag1=0;
				for(k=0;k<sym_count;k++)
				{

					if(strcmp(temp_operand,symList[k].label)==0)
					{
						flag1=1;
						break;
					}
				}

				if(inst.opcode[0]=='+')
				{
					object_code |=1;
				}
				else
				{
					if(inst.opcode[0]!='+' && flag1)
						object_code |=2;
				}

				if(flag1)
				{
					if(inst.opcode[0]!='+')
					{
						object_code=object_code<<12;
						inst1=*(instr+i+1);
						if(inst1.loc < symList[k].loc)
						{
							object_code|=symList[k].loc-inst1.loc;
							//object_code|=(~(inst1.loc-symList[k].loc)+1)&0xfff;
						}
						else
						{
							if(strcmp(inst.operand,base)==0)
							{
								object_code &=0xff0000;
								object_code |=0x4000;
								//printf("label: %s opcode: %s operand: %s object_code: %x\n",inst.label,inst.opcode,inst.operand,object_code);	
							}
							else
							{
								object_code|=(~(inst1.loc-symList[k].loc)+1)&0xfff;
							}
							//object_code|=symList[k].loc-inst1.loc;
						}

					}
					else
					{
						object_code=object_code<<20;
						object_code|=symList[k].loc;
					}
				}
				else if(inst.operand[0]=='#')
				{

					if(inst.opcode[0]!='+')
					{
						object_code=object_code<<12;
						object_code|=atoi(temp_operand);
					}
					else
					{
						object_code=object_code<<20;
						object_code|=atoi(temp_operand);
					}
				}
				else
				{	
					if(inst.operand[strlen(inst.operand)-1]=='X')
						object_code|=0xc;
					object_code=object_code<<12;
				}
			}
		}

		inst.object_code=object_code;
		*(instr+i)=inst;
	}
	display(instr,line);
	object_program(instr,line);
}


void display(struct instruction *instr, int line)
{

	struct instruction inst;
	int i;
	char blank[2];
	blank[0]='\0';
	printf("\n\n--------------------Display file-------------------\n\n");
	for(i=0;i<line;i++)
	{
		inst=*(instr+i);
		if(strcmp(inst.opcode,BASE)==0)
		{
			printf("%-4s\t%-10s%-10s%-10s\t\n",blank,inst.label,inst.opcode,inst.operand);
			continue;
		}
		else if(strcmp(inst.opcode,"CLEAR")==0 || strcmp(inst.opcode,"COMPR")==0 || strcmp(inst.opcode,"TIXR")==0)
		{
			printf("%04x\t%-10s%-10s%-10s\t%04x\n",inst.loc,inst.label,inst.opcode,inst.operand,inst.object_code);
		}
		else if(strcmp(inst.opcode,END)==0)
		{
			printf("%-4s\t%-10s%-10s%-10s\t\n",blank,inst.label,inst.opcode,inst.operand);
			continue;
		}
		else if(inst.object_code==0)
		{
			printf("%04x\t%-10s%-10s%-10s\t\n",inst.loc,inst.label,inst.opcode,inst.operand);
			continue;
		}
		else if(strcmp(inst.opcode,BYTE)==0)
		{

			printf("%04x\t%-10s%-10s%-10s\t%02x\n",inst.loc,inst.label,inst.opcode,inst.operand,inst.object_code);
			continue;
		}
		else
		{
			printf("%04x\t%-10s%-10s%-10s\t%06x\n",inst.loc,inst.label,inst.opcode,inst.operand,inst.object_code);
		}
	}
	printf("------\n\n");
}

void object_program(struct instruction *instr, int line)
{
	struct instruction inst,inst1;
	int i=0,length=0,inc=0,flag=0,cc=0,cp=0,pw=0,ss=0;
	char blank[2];
	int f;
	FILE *fp;
	fp=fopen("object.txt","w");
	blank[0]='\0';


	inst=*(instr+i);
	inst1=*(instr+(line-1));
	fprintf(fp,"H^%-6s^%06x^%06x\n",inst.label,inst.loc,inst1.loc-inst.loc);
	f=inst.loc;
	for(i=1;i<line;i++)
	{
		inst=*(instr+i);
		if(inst.object_code==0 && strcmp(inst.opcode,"BASE")!=0)
		{
			flag=0;
			continue;
		}
		if(!ss)
		{
			if(inst.object_code!=0)
			{
				fprintf(fp,"\nT^%06x^00",inst.loc);
				pw=ftell(fp);
				flag=1;
				cc=0;
				fprintf(fp,"^%06x",inst.object_code);
				cc++;
				ss=1;
			}
			continue;
		}

		if(strcmp(inst.opcode,BASE)==0)
		{
			//printf("%-4s\t%-10s%-10s%-10s\t\n",blank,inst.label,inst.opcode,inst.operand);
			continue;
		}
		else if(strcmp(inst.opcode,"CLEAR")==0 || strcmp(inst.opcode,"COMPR")==0 || strcmp(inst.opcode,"TIXR")==0)
		{
			//printf("%04x\t%-10s%-10s%-10s\t%04x\n",inst.loc,inst.label,inst.opcode,inst.operand,inst.object_code);


			cp=ftell(fp);
			//fprintf(fp,"^%x",inst.object_code);
			if((ftell(fp)+5-pw-cc)<=60 && flag==1)
			{
				//fseek(fp,cp,SEEK_SET);
				fprintf(fp,"^%x",inst.object_code);
				cc++;
			}
			else
			{

				//fseek(fp,cp,SEEK_SET);
				if(inst.object_code!=0)
				{
					fseek(fp,pw-2,SEEK_SET);
					if((cp-pw-cc)%2==0)
						fprintf(fp,"%02x",(cp-pw-cc)/2);
					else
						fprintf(fp,"%02x",((cp-pw-cc)/2)+1);
					fseek(fp,0,SEEK_END);

					fprintf(fp,"\nT^%06x^00",inst.loc);
					pw=ftell(fp);
					flag=1;
					cc=0;
					fprintf(fp,"^%04x",inst.object_code);
					cc++;
				}
			}


		}
		else if(strcmp(inst.opcode,END)==0)
		{
			//printf("%-4s\t%-10s%-10s%-10s\t\n",blank,inst.label,inst.opcode,inst.operand);
		}
		else if(inst.object_code==0)
		{
			//printf("%04x\t%-10s%-10s%-10s\t\n",inst.loc,inst.label,inst.opcode,inst.operand);
			flag=0;
			continue;
		}
		else if(strcmp(inst.opcode,BYTE)==0)
		{

			//printf("%04x\t%-10s%-10s%-10s\t%02x\n",inst.loc,inst.label,inst.opcode,inst.operand,inst.object_code);



			cp=ftell(fp);
			//fprintf(fp,"^%x",inst.object_code);
			if((ftell(fp)+code_len(inst.object_code)-pw-cc)<=60 && flag==1)
			{
				fprintf(fp,"^%x",inst.object_code);
				cc++;
			}
			else
			{

				//fseek(fp,cp,SEEK_SET);
				if(inst.object_code!=0)
				{
					fseek(fp,pw-2,SEEK_SET);
					if((cp-pw-cc)%2==0)
						fprintf(fp,"%02x",(cp-pw-cc)/2);
					else
						fprintf(fp,"%02x",((cp-pw-cc)/2)+1);
					fseek(fp,0,SEEK_END);
					fprintf(fp,"\nT^%06x^00",inst.loc);
					pw=ftell(fp);
					flag=1;
					cc=0;
					fprintf(fp,"^%x",inst.object_code);
					cc++;
				}
			}
			continue;
		}
		else
		{
			//printf("%04x\t%-10s%-10s%-10s\t%06x\n",inst.loc,inst.label,inst.opcode,inst.operand,inst.object_code);
			cp=ftell(fp);
			
			//fprintf(fp,"^%06x",inst.object_code);
			if((ftell(fp)+7-pw-cc)<=60 && flag==1)
			{
				//fseek(fp,cp,SEEK_SET);
				fprintf(fp,"^%06x",inst.object_code);
				cc++;
			}
			else
			{

				//fseek(fp,cp,SEEK_SET);
				if(inst.object_code!=0)
				{
					fseek(fp,pw-2,SEEK_SET);
					if((cp-pw-cc)%2==0)
						fprintf(fp,"%02x",(cp-pw-cc)/2);
					else
						fprintf(fp,"%02x",((cp-pw-cc)/2)+1);



					fseek(fp,0,SEEK_END);
					fprintf(fp,"\nT^%06x^00",inst.loc);
					pw=ftell(fp);
					flag=1;
					cc=0;
					fprintf(fp,"^%06x",inst.object_code);
					cc++;
				}
			}
		}
	}


	cp=ftell(fp);
	fseek(fp,pw-2,SEEK_SET);

	if((cp-pw-cc)%2==0)
		fprintf(fp,"%02x",(cp-pw-cc)/2);
	else
		fprintf(fp,"%02x",((cp-pw-cc)/2)+1);
	fseek(fp,0,SEEK_END);

	fprintf(fp,"\nE^%06x\n",f);
	fclose(fp);


}

char *truncate(char ch[16])
{
	int i;
	for(i=0;i<strlen(ch)-1;i++)
	{
		ch[i]=ch[i+1];
	}
	ch[i]='\0';
	return(ch);
}
int code_len(int x)
{
  if(x<=0xf)
  {
     x=1;
  }
  else if(x<=0xff)
  {
  	x=2;
  }
  else if(x<=0xfff)
  {
  	x=3;
  }
  else if(x<=0xffff)
  {
  	x=4;
  }
  else if(x<=0xfffff)
  {
  	x=5;
  }
  else if(x<=0xffffff)
  {
  	x=6;
  }
  else if(x<=0xfffffff)
  {
  	x=7;
  }
  else if(x<=0xffffffff)
  {
  	x=8;
  }
  else if(x<=0xfffffffff)
  {
  	x=9;
  }
  else
  {
	  x=10;
  }
  return(x);
}
