#include<iostream>
#include<fstream>
#include<cstdlib>
#include<math.h>
#include<stdlib.h>
#include <chrono> 

using namespace std;
using namespace std::chrono;
string instructions[100]; //save the instructios
int mem[5]; // simulated the memory
int reg[10]; //simulated the register
int clockcycle=0; // simulated the clock cycle
ofstream fout("Result.txt");

///deal with hazard
bool lwhz=false;
bool bneq=false;
bool flu=false;

///for instruction fetch variable
int pc = 0;
bool ifid_input = false;
string finstruction="00000000000000000000000000000000";

///for insruction decode and register fetch variable
int idpc=0;
string op="000000";
string fun="000000";
string idexsignal="000000000";
bool idex_input=0;
int readdata1=0;
int readdata2=0;
int signextend=0;
long int rs=0;
int rt=0;
int rd=0;

///for execution variable
bool exe_input=false;
int aluout=0;
int writedata=0;
int exert=0;
int exepc=0;
int temp=0;
int temp2=0;
string exesignal="000000000";
string aluop="00";

///for read or write memory variable
bool meminput=false;
bool membranch=false;
int memreaddata=0;
int memalu=0;
int memrt=0;
string memsignal="00";

/// for wb
int wbrd=0;


void writeback(string wbsignal, int rt , int alu, int readata)
{
    if (rt ==0)//$0 cant overwrite
        return;
    if(wbsignal[0]=='1')
    {
        if(wbsignal[1]=='0')//R-type
        {
            reg[rt]=alu;
        }
        else if(wbsignal[1]=='1')//lw
        {
            reg[rt]=readata;
        }
    }
    else if(wbsignal[0]=='0')//sw and beq
    {
        //NOP
        return ;
    }
}
void instructiondecode()
{
    idex_input=ifid_input;
    ifid_input=false;
    idpc=pc;
    op=finstruction.substr(0,6);//first 6 bits
    if(op=="000000")//r type
    {
        rs=b_d(finstruction.substr(6,5),false);
        rt=b_d(finstruction.substr(11,5),false);
        rd=b_d(finstruction.substr(16,5),false);
        fun=finstruction.substr(26);
        readdata1=reg[rs];
        readdata2=reg[rt];
        //r type no singextend
        signextend=b_d(finstruction.substr(16,16),true);
        if(rs==0&&rt==0&&rd==0&&fun=="000000"||lwhz==true)//no instruction
        {
            idexsignal="000000000";
            lwhz=false;

        }
        else
        {
            idexsignal="110000010";
        }

    }
    else//i type
    {
        rs=b_d(finstruction.substr(6,5),false);
        rt=b_d(finstruction.substr(11,5),false);
        readdata1=reg[rs];
        readdata2=reg[rt];
        signextend=b_d(finstruction.substr(16,16),true);
        //i type no
        rd=0;
        if(op=="100011")//lw
        {
            idexsignal="000101011";
        }
        else if(op=="101011")//sw
        {
            idexsignal="100100101";
        }
        else if(op=="001000")//addi
        {
            idexsignal="000100010";
        }
        else if(op=="001100")//andi
        {
            idexsignal="011100010";
        }
        else if(op=="000101")//bne
        {
            idexsignal="101010001";
        }
    }
}
void execution()
{
    exe_input=idex_input;
    idex_input=false;

    aluop=idexsignal.substr(1,2);
    exesignal=idexsignal.substr(4,5);
    writedata=readdata2;//for sw, not always use
    if(idexsignal[0]=='1'&&idexsignal[1]=='1')//r type
    {
        exert=rd;
    }
    else
    {
        exert=rt;
    }
    temp=readdata1;
    if(idexsignal[3]=='0')//alusrc== 0
        temp2=readdata2;
    else //alusrc==0
        temp2=signextend;
    if(aluop=="10")//r type
    {

        if(fun=="100000" )//add
            aluout=temp+temp2;
        else if(fun== "100100" )//and
            aluout=temp&temp2;
        else if(fun== "100101" )//or
            aluout=temp|temp2;
        else if(fun== "100010" )//sub
            aluout=temp-temp2;
        else if(fun== "101010" )//slt
        {
            if (temp<temp2)
                aluout=1;
            else
                aluout=0;
        }
        else if(fun== "000000" )
            aluout=0;
    }
    else if(aluop=="00")//lw&sw
        aluout=temp+temp2;
    else if(aluop=="01")//bNeq
    {
        aluout=temp-temp2;
        bneq=true;
        if(aluout!=0)
            flu=true;
    }

    else if(idexsignal=="000100010")//addi
    {
        aluout=temp+temp2;

    }
    else if(idexsignal=="011100010")//andi
    {
        aluout=temp&temp2;
    }
    exepc=idpc+(signextend*4);

}

void memoryaccess()
{
    meminput=exe_input;
    exe_input=false;
    membranch=false;
    memrt=exert;
    memalu=aluout;
    memsignal=exesignal.substr(3,2);
    if (memsignal == "01" && memalu != 0&&bneq==true) //bneq
    {
        bneq=false;
        membranch = true;
    }
    if(memsignal == "11")    //load
    {
        memreaddata=mem[memalu/4];
    }
    else
    {
        memreaddata=0;
    }
    if(memsignal == "01"&&exesignal[2]=='1')//sw
    {
        mem[memalu/4]=writedata;
    }

}

void instructionfetch()
{
    pc=pc+4;
    ifid_input=true;
    if(instructions[(pc/4)]=="")// pc/4= the number of instruction
    {
        finstruction="00000000000000000000000000000000";//no instruction
        ifid_input=false;
    }
    else
    {
        finstruction=instructions[(pc/4)];//fetch the instruction
    }
}

