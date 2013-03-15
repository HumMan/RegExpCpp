#include "RegExp.h"



void TNFA::AddBranch(TFANode* node,TFANode* use_target,TBranchType use_branch_type,char use_min,char use_max)
{
	TFANode* temp;
	if ((use_branch_type==BT_SUBRANGE)&&(use_min==use_max))
		use_branch_type=BT_CHAR;
	assert((use_branch_type!=BT_NOT_NEW_LINE)&&(use_branch_type!=BT_NOT_SPACE));
	//символ перевода строки является последовательностью двух символов
	if(use_branch_type==BT_NEW_LINE)
	{
		temp=nodes.Add();
		AddBranch(node,temp,BT_CHAR,13);
		AddBranch(temp,use_target,BT_CHAR,10);
	}
	else if (use_branch_type==BT_SPACE)
	{
		temp=nodes.Add();
		AddBranch(node,temp,BT_CHAR,13);
		AddBranch(temp,use_target,BT_CHAR,10);
		AddBranch(node,use_target,BT_CHAR,9);
		AddBranch(node,use_target,BT_CHAR,' ');
	}
	//
	else
	{
		node->branches.Add(TBranch(use_branch_type,use_min,use_max,use_target));
		use_target->branches_in.Add(TBranch(use_branch_type,use_min,use_max,node));
	}
}

template<class T>
void Swap(TArray<T> &v1,TArray<T> &v2)
{
	assert(v1.GetHigh()==v2.GetHigh());
	T temp;
	for(int i=0;i<=v1.GetHigh();i++)
	{
		temp=v1[i];v1[i]=v2[i];v2[i]=temp;
	}
}

////----------------------------------------------------------------------------//
//
////--TNFA----------------------------------------------------------------------//
//

void TNFA::GetTerm(bool alternatives_mode,TBranchType &btype,char &val)
{
	int t;
	val=0;
	switch(expr[curr_char])
	{
	case '?':
	case '+':
	case '*':
	case '(':
	case ')':
	case '|':
	case '[':
	case ']':
	case '{':
	case '}':
		throw "Ошибка в выражении!";
		break;
	case '#':
		t=0;
		if((curr_char<expr_length-1)&&(IsIn((char)expr[curr_char+1],'0','9'))) 
		{
			curr_char++;
			while((curr_char<expr_length)&&IsIn((char)expr[curr_char],'0','9'))
			{
				t=t*10+(expr[curr_char]-'0');
				curr_char++;
			}
		}
		else throw "Неожиданный конец выражения после ""#""!";
		btype=BT_CHAR;
		val=t;
		if(t==13)throw "Сивол #13 используется только в ""\\n"" и ""\\N""!";
		break;
	case '.':
		if(alternatives_mode)
		{
			btype=BT_CHAR;
			val='.';
		}
		else btype=BT_ANY_CHAR;
		curr_char++;
		break;
	case '\\':
		curr_char++;
		if(alternatives_mode &&
			(
			(char)expr[curr_char]=='S'||
			(char)expr[curr_char]=='W'||
			(char)expr[curr_char]=='D'||
			(char)expr[curr_char]=='N'||
			(char)expr[curr_char]=='T'))
			throw "Классы символов \\S, \\W, \\D, \\N, \\T нельзя использовать в качестве альтернатив!";
		if (curr_char<expr_length) 
		{
			switch(expr[curr_char])
			{
			case 'S':btype=BT_NOT_SPACE;break;
			case 'W':btype=BT_NOT_WORD;break;
			case 'D':btype=BT_NOT_DIGIT;break;
			case 'N':btype=BT_NOT_NEW_LINE;break;
			case 'T':btype=BT_NOT_TAB;break;
			case 's':btype=BT_SPACE;break;
			case 'w':btype=BT_WORD;break;
			case 'd':btype=BT_DIGIT;break;
			case 'n':btype=BT_NEW_LINE;break;
			case 't':btype=BT_TAB;break;
			case '\\':
			case '.':
			case '?':
			case '+':
			case '*':
			case '(':
			case ')':
			case '|':
			case '[':
			case ']':
			case '#':
			case '{':
			case '}':
			case '\'':
				btype=BT_CHAR;
				val=expr[curr_char];
				break;
			default: throw "Неизвестный символ после ""\\""!";
			}
			curr_char++;
		}else throw "Неожиданный конец выражения после ""\\""!";
		break;
	default:
		{
			btype=BT_CHAR;
			val=expr[curr_char];
			curr_char++;
		}
	}
}

void GetRanges(TArray<TCharRange> &subranges, TBranchType btype, char min=0, char max=0)
{
	char val_temp;
	switch(btype)
	{
	case BT_CHAR:
	case BT_NEW_LINE:
	case BT_TAB:
		switch(btype)
		{
		case BT_CHAR:val_temp=min;break;
		case BT_NEW_LINE:val_temp=13;break;
		case BT_TAB:val_temp=9;break;
		default:val_temp=0;
		}
		subranges.Add(TCharRange(val_temp));
		break;
	case BT_SPACE:
		subranges.Add(TCharRange(' '));
		subranges.Add(TCharRange(9));
		subranges.Add(TCharRange(13));
		break;
	case BT_WORD:
		subranges.Add(TCharRange('0','9'));
		subranges.Add(TCharRange('A','Z'));
		subranges.Add(TCharRange('a','z'));
		subranges.Add(TCharRange('А','Я'));
		subranges.Add(TCharRange('а','я'));
		break;
	case  BT_DIGIT:
		subranges.Add(TCharRange('0','9'));
		break;
	case BT_SUBRANGE:
		assert(char(max)>char(min));
		subranges.Add(TCharRange(min,max));
		break;
	case BT_ANY_CHAR:
		subranges.Add(TCharRange(0,255));
		break;
	case BT_NOT_CHAR:
		if (min>0)   subranges.Add(TCharRange(0,min-1)); 
		if (min<255) subranges.Add(TCharRange(min+1,255)); 
		break;
	default:
		assert(false);
	}
}

void TNFA::BuildAlternatives(TFANode* start,TFANode* finish)
{
	TBranchType btype;
	char val1,val2,val_temp;
	bool not_alternative,need_search;
	TArray<TCharRange> subranges;// используется для получения инвертирования группы диапазонов ( not(r1 || r2 || ... rn)
	int subranges_high,curr_min,curr_max;
	TFANode* temp;
	bool add_new_line_alt=false;
	curr_char++;
	if (curr_char<expr_length&&(char)expr[curr_char]=='^')
	{
		not_alternative=true;
		curr_char++;
	} else not_alternative=false;
	//считываем все альтернативы в subranges, преобразуем в диапазоны и оптимизируем при not_alternative
	subranges_high=-1;

	while(curr_char<expr_length && (char)expr[curr_char]!=']')
	{
		GetTerm(true,btype,val1);
		//Если диапазон символов
		if((curr_char<expr_length)&&((char)expr[curr_char]=='-'))
		{
			if(btype!=BT_CHAR)throw "Оператор диапазона ""-"" применяется только к символам!";
			curr_char++;
			if ((curr_char>=expr_length) || ((char)expr[curr_char]==']') )
				throw "Отсутствует второй операнд оператора ""-""!";
			GetTerm(true,btype,val2);
			if (btype!=BT_CHAR)
				throw "Оператор диапазона ""-"" применяется только к символам!";
			if (val1>val2)
			{ val_temp=val1;val1=val2;val2=val_temp; }
			//Если отрицательная альтернатива
			if (not_alternative )
				subranges.Add(TCharRange(val1,val2));
			//Если не отрицательная альтернатива
			else AddBranch(start,finish,BT_SUBRANGE,val1,val2);
		}
		//Если не диапазон и отрицательная альтернатива"[^"
		else if (not_alternative )
		{
			if((btype=BT_NEW_LINE)||(btype=BT_SPACE))  add_new_line_alt=true;
			GetRanges(subranges,btype);
		}
		//Если не диапазон и не отрицательная альтернатива
		else AddBranch(start,finish,btype,val1);
	}
	//subranges содержит запрещенные диапазоны символов
	//поэтому находим не запрещенные участки и добавляем в переходы
	if(not_alternative)
	{
		curr_min=0;
		curr_max=255;
		while(curr_min<=255)
		{
			need_search=true;
			while(need_search)
			{
				need_search=false;
				for(int i=0;i<=subranges.GetHigh();i++)
				{
					if(IsIn(char(curr_min),subranges[i].min,subranges[i].max))
					{
						curr_min=subranges[i].max+1;
						need_search=true;
					}
				}
			}
			if(curr_min==256) break;
			for(int i=0;i<=subranges.GetHigh();i++)
				if (subranges[i].min>curr_min && subranges[i].min<=curr_max )
					curr_max=subranges[i].min-1;
			//добавляем диапазон в branch
			if (curr_min==curr_max)
				AddBranch(start,finish,BT_CHAR,curr_min);
			else
				AddBranch(start,finish,BT_SUBRANGE,curr_min,curr_max);
			curr_min=curr_max+1;
			curr_max=255;
		}
		//т.к. перевод строки представляется двумя символами(#13#10),
		//то сначала проверяем не запрещен ли уже #13, а затем добавляем обработку #10
		if (add_new_line_alt )
		{
			temp=nodes.Add();
			AddBranch(start,temp,BT_CHAR,13);
			AddBranch(temp,finish,BT_NOT_CHAR,10);
		}
	}
}

void TNFA::AddAlternativeRegister(TFANode* start,TFANode* finish)
{
	AddNodeAlternativeRegister(start,finish);
	for(int i=0;i<=nodes.GetHigh();i++)
		nodes[i]->flag=false;
}

//TODO: преобразование регистров для русских букв
bool GetAlternativeRegister(char c1,char c2,char &res1, char &res2,bool to_upper)
{
	bool result=false;
	char t1,t2;
	assert(c1<c2);
	if(to_upper)
	{
		if (c1<'a')  t1='a'; else t1=c1;
		if (c2>'z')  t2='z'; else t2=c2;
		if (t1<=t2) 
		{
			result=true;
			res1=char(t1-'a'+'A');
			res2=char(t2-'a'+'A');
		}
	}
	else
	{
		if (c1<'A')  t1='A'; else t1=c1;
		if (c2>'Z')  t2='Z'; else t2=c2;
		if (t1<=t2) 
		{
			result=true;
			res1=char(t1+'a'-'A');
			res2=char(t2+'a'-'A');
		}
	}
	return result;
}

bool GetAlternativeRegister(char c,char &res)
{
	if     (IsIn(c,'a','z'))res=char(c-'a'+'A');
	else if(IsIn(c,'A','Z'))res=char(c+'a'-'A');
	else if(IsIn(c,'а','я'))res=char(c-'я'+'Я');
	else if(IsIn(c,'А','Я'))res=char(c+'я'-'Я');
	else return false;
	return true;
}

void TNFA::AddNodeAlternativeRegister(TFANode* node,TFANode* finish)

{
	if(!node->flag&&node!=finish)
	{
		node->flag=true;
		int h=node->branches.GetHigh();
		for(int i=0;i<=h;i++)
		{
			char c1,c2;
			switch(node->branches[i].branch_type)
			{
			case BT_CHAR:
				if (GetAlternativeRegister(node->branches[i].min,c1))
					AddBranch(node,node->branches[i].target_node,BT_CHAR,c1);
				break;
			case BT_SUBRANGE:
				if (GetAlternativeRegister(node->branches[i].min,node->branches[i].max,c1,c2,false)) 
					AddBranch(node,node->branches[i].target_node,BT_SUBRANGE,c1,c2);
				if (GetAlternativeRegister(node->branches[i].min,node->branches[i].max,c1,c2,true)) 
					AddBranch(node,node->branches[i].target_node,BT_SUBRANGE,c1,c2);
				break;
			}
			AddNodeAlternativeRegister(node->branches[i].target_node,finish);
		}
	}
}

void TNFA::BuildNFA(TFANode* start,TFANode* finish,int level)
{
	TFANode *temp,*temp1,*temp2,*temp3;
	TBranchType btype;
	char val1;
	switch(level)
	{
	case 0://оператор |
		BuildNFA(start,finish,1);
		while ((curr_char<expr_length)&&(expr[curr_char]=='|'))
		{
			curr_char++;
			BuildNFA(start,finish,1);
		}
		break;
	case 1://конкатезация
		//прерывается если встречается более низкий по приоритету оператор или закрывающая скобка
		while(curr_char<expr_length && expr[curr_char]!='|' && expr[curr_char]!=')')
		{
			temp=nodes.Add();
			BuildNFA(start,temp,2);
			start=temp;
		}
		if(curr_char<expr_length && expr[curr_char]==')' && parenthnes_level==0)
			throw "Лишняя закрывающая круглая скобка!";
		AddBranch(start,finish);
		break;
	case 2:// операторы * + ?

		temp=nodes.Add();
		temp1=nodes.Add();
		temp2=nodes.Add();
		temp3=nodes.Add();
		AddBranch(temp,temp2);
		AddBranch(temp2,finish);
		AddBranch(start,temp3);
		AddBranch(temp3,temp1);
		BuildNFA(temp1,temp,3);
		if (curr_char<expr_length)
		{
			switch(expr[curr_char])
		 {
			case '*':                                 //                _______________
				//               \/              |    e
				curr_char++;                  //(s)->temp3-> (temp1) ..... (temp) --> temp2 -> (f)
				AddBranch(temp1,temp2);            //               |                      /
				AddBranch(temp,temp1);             //               |_______________________|
				break;                                 //                   e
			case '+':
				curr_char++;
				AddBranch(temp,temp1);
				break;
			case '?':
				curr_char++;
				AddBranch(temp1,temp2);
				break;
			case '{':
				curr_char++;
				while (curr_char<expr_length && (expr[curr_char]=='r' || expr[curr_char]=='s'))
				{
					switch(expr[curr_char])
					{
					case 's':
						curr_char++;
						AddBranch(temp3,temp3,BT_SPACE);
						AddBranch(temp2,temp2,BT_SPACE);
						break;
					case 'r':
						curr_char++;
						AddAlternativeRegister(temp1,temp2); 
						break;
					}
				}
				if(curr_char>=expr_length || expr[curr_char]!='}')
					throw "Ожидалась закрывающая фигурная скобка!";
				curr_char++;
				break;
			}
		}
		break;
	case 3:

		if (curr_char<expr_length) 
		{
			//
			switch(expr[curr_char])
			{
			case '['://альтернативы []
				BuildAlternatives(start,finish);
				if (curr_char<expr_length && expr[curr_char]==']')curr_char++;
				else throw "Ожидалась закрывающая квадратная скобка!";
				break;
			case '('://скобки ()
				parenthnes_level++;
				curr_char++;
				BuildNFA(start,finish,0);
				if (curr_char<expr_length && expr[curr_char]==')')curr_char++;
				else throw "Ожидалась закрывающая круглая скобка!";
				parenthnes_level--;
				break;
			default:
				GetTerm(false,btype,val1);
				if (btype==BT_NOT_SPACE)
				{
					temp=nodes.Add();
					AddBranch(start,finish,BT_SUBRANGE,0,8);
					AddBranch(start,finish,BT_SUBRANGE,10,12);
					AddBranch(start,finish,BT_SUBRANGE,14,31);
					AddBranch(start,finish,BT_SUBRANGE,33,255);
					AddBranch(start,temp,BT_CHAR,13);
					AddBranch(temp,finish,BT_NOT_CHAR,10);
				}
				else if (btype==BT_NOT_NEW_LINE)
				{
					temp=nodes.Add();
					AddBranch(start,finish,BT_NOT_CHAR,13);
					AddBranch(start,temp,BT_CHAR,13);
					AddBranch(temp,finish,BT_NOT_CHAR,10);
				}
				else AddBranch(start,finish,btype,val1);
			}
			//
		}
		break;
	}
}

inline bool IsWord(char a)
{
	return IsIn(a,'0','9')||IsIn(a,'A','Z')||IsIn(a,'a','z')||IsIn(a,'А','Я')||IsIn(a,'а','я');
}

bool TNFA::Move(char a)
//##############################################################################
//TNFA.Move вычисляет все состояния автомата после чтения входного символа
//in:
//  a - входной символ
//  curr_states - текущие состояния автомата
//out:
//  curr_states - состояния после чтения
//temp:
//  result_states, result_bit_vector
//##############################################################################
{
	TArray<TFANode*> *temp;
	bool r;
	result_states_high=-1;
	bool result=true;
	for(int i=0;i<=curr_states_high;i++)
	{
		int h=(*curr_states)[i]->branches.GetHigh();
		for(int k=0;k<=h;k++)
		{
			r=false;
			switch((*curr_states)[i]->branches[k].branch_type)
			{
			case BT_CHAR: r= a==(*curr_states)[i]->branches[k].min;break;
			case BT_WORD: 
				r=IsWord(a);break;
			case BT_DIGIT:r=IsIn(a,'0','9');break;
			case BT_SUBRANGE: r= IsIn(a,(*curr_states)[i]->branches[k].min,(*curr_states)[i]->branches[k].max);break;
			case BT_NOT_CHAR: r= a!=(*curr_states)[i]->branches[k].min;break;
			case BT_NOT_WORD: r=!IsWord(a);break;
			case BT_NOT_DIGIT: r= !IsIn(a,'0','9');break;
			case BT_ANY_CHAR: r=true;break;
			case BT_TAB: r= a==9;break;
			case BT_EMPTY: r=false;break;
			default: assert(false);
			}
			if ( r && !result_bit_vector[(*curr_states)[i]->branches[k].target_node->index]) 
			{
				result_states_high++;
				(*result_states)[result_states_high]=(*curr_states)[i]->branches[k].target_node;
				result_bit_vector[(*curr_states)[i]->branches[k].target_node->index]=true;
			}
		}
	}
	if (result_states_high==-1)return false;
	temp=curr_states;curr_states=result_states;result_states=temp;
	curr_states_high=result_states_high;
	memset(&result_bit_vector[0],0,nodes.GetHigh()+1);

	return result;
}

void TNFA::EClosure(bool except_unimportant_nodes)
//##############################################################################
//TNFA.EClosure вычисляет E замыкание автомата
//in:
//  curr_states - текущие состояния автомата
//out:
//  curr_states - состояния после E замыкания
//temp:
//  result_states, result_bit_vector
//##############################################################################
{
	TArray<TFANode*> *temp;
	TFANode *t,*b;
	memset(&result_bit_vector[0],0,nodes.GetCount());
	//
	for(int i=0;i<=curr_states_high;i++)
		result_bit_vector[(*curr_states)[i]->index]=true;
	//
	for(int i=0;i<=curr_states_high;i++)
		(*result_states)[i]=(*curr_states)[i];
	result_states_high=curr_states_high;
	while (curr_states_high>=0)
	{
		t=(*curr_states)[curr_states_high];
		curr_states_high--;
		for(int i=0;i<=t->branches.GetHigh();i++)
			if (t->branches[i].branch_type==BT_EMPTY )
			{
				b=t->branches[i].target_node;
				if (!result_bit_vector[b->index])
				{
					curr_states_high++;
					(*curr_states)[curr_states_high]=b;
					result_states_high++;
					(*result_states)[result_states_high]=b;
					result_bit_vector[b->index]=true;
				}
			}
	}
	//
	memset(&result_bit_vector[0],0,nodes.GetCount());
	//
	//Swap(curr_states,result_states);
	temp=curr_states;curr_states=result_states;result_states=temp;
	curr_states_high=result_states_high;
	//
	if(except_unimportant_nodes)
		//этот блок исользуется в ДКА для исключения неважных состояний(имеющих только E-переходы)
	{
		result_states_high=-1;
		for(int i=0;i<=curr_states_high;i++)
		{
			//конечное состояние пропускаем без проверки
			if((*curr_states)[i]==finish) goto end_search;
			//
			for(int k=0;k<=(*curr_states)[i]->branches.GetHigh();k++)
				if((*curr_states)[i]->branches[k].branch_type!=BT_EMPTY)  goto end_search;
			continue;
end_search:
			result_states_high++;
			(*result_states)[result_states_high]=(*curr_states)[i];
		}
		temp=curr_states;curr_states=result_states;result_states=temp;
		curr_states_high=result_states_high;
	}
}

bool TNFA::Match(char* s)
{
	int result=false;
	int s_len=strlen(s);
	if(s_len==-1 || start==NULL) return false;
	curr_char=0;
	memset(&result_bit_vector[0],0,nodes.GetCount());
	//
	curr_states_high=0;
	(*curr_states)[0]=start;
	EClosure();
	while (curr_char<s_len)
	{
		if (!Move(s[curr_char])) return false;
		EClosure();
		curr_char++;
	}
	for(int i=0;i<=curr_states_high;i++)
		if ((*curr_states)[i]==finish) return true;
	return result;
}

void TNFA::ReplaceBranch(TArray<TBranch> &branches,TFANode* branch,TBranch new_val)
{
	for(int i=0;i<=branches.GetHigh();i++)
		if (branches[i].target_node==branch)
			branches[i]=new_val;
}

void TNFA::ReplaceBranch(TArray<TBranch> &branches,TFANode* branch,TFANode* new_val)
{
	for(int i=0;i<=branches.GetHigh();i++)
		if (branches[i].target_node==branch)
			branches[i].target_node=new_val;
}

void TNFA::Optimize()
{
	for(int i=0;i<=nodes.GetHigh();i++)
	{
		//Входящий и исходящий переход пуст (является E переходом)
		if(nodes[i]->branches_in.GetHigh()==0 && nodes[i]->branches.GetHigh()==0
			&& nodes[i]->branches_in[0].branch_type==BT_EMPTY
			&& nodes[i]->branches[0].branch_type==BT_EMPTY)
		{
			TFANode* n=nodes[i]->branches_in[0].target_node;
			ReplaceBranch(n->branches,nodes[i],nodes[i]->branches[0].target_node);
			ReplaceBranch(nodes[i]->branches[0].target_node->branches_in,nodes[i],n);
			nodes.Del(i);
			i--;
		}
		else
			if (nodes[i]->branches_in.GetHigh()==0 && 
				nodes[i]->branches_in[0].branch_type==BT_EMPTY&&
				nodes[i]->branches.GetHigh()>=0)
			{
				for(int t=0;t<=nodes[i]->branches.GetHigh();t++)
					if (nodes[i]->branches[t].branch_type==BT_EMPTY)  goto end_search1;
				//Входной переход пуст, а все исходящие переходы не пусты
				TFANode* n=nodes[i]->branches_in[0].target_node;
				for(int t=0;t<=nodes[i]->branches.GetHigh();t++)
					ReplaceBranch(nodes[i]->branches[t].target_node->branches_in,nodes[i],n);
				ReplaceBranch(n->branches,nodes[i],nodes[i]->branches[0]);
				int e=n->branches.GetCount();
				n->branches.SetCount(e+nodes[i]->branches.GetHigh());
				for(int t=1;t<=nodes[i]->branches.GetHigh();t++)
					n->branches[e+t-1]=nodes[i]->branches[t];
				nodes.Del(i);
				i--;
end_search1:
				int q;
			}
			else

				if (nodes[i]->branches.GetHigh()==0 && 
					nodes[i]->branches[0].branch_type==BT_EMPTY &&
					nodes[i]->branches_in.GetHigh()>=0)
				{
					for(int t=0;t<=nodes[i]->branches_in.GetHigh();t++)
						if (nodes[i]->branches_in[t].branch_type==BT_EMPTY) goto end_search2;
					//Все входные переходы не пусты, а единственный исходящий переход пуст
					TFANode* n=nodes[i]->branches[0].target_node;
					for(int t=0;t<=nodes[i]->branches_in.GetHigh();t++)
						ReplaceBranch(nodes[i]->branches_in[t].target_node->branches,nodes[i],n);
					ReplaceBranch(n->branches_in,nodes[i],nodes[i]->branches_in[0]);
					int e=n->branches_in.GetCount();
					n->branches_in.SetCount(e+nodes[i]->branches_in.GetHigh());
					for(int t=0;t<=nodes[i]->branches_in.GetHigh();t++)
						n->branches_in[e+t-1]=nodes[i]->branches_in[t];
					nodes.Del(i);
					i--;
end_search2:
					int q;
				}

	}
}

void TNFA::PrepareBitVector()
{
	for(int i=0;i<=nodes.GetHigh();i++)nodes[i]->index=i;
}

void TNFA::SetExpression(char* s)
{
	expr=s;
	expr_length=strlen(s);
	curr_char=0;
	try
	{
		start=nodes.Add();
		finish=nodes.Add();
		parenthnes_level=0;
		BuildNFA(start,finish,0);
		Optimize();
		int l=nodes.GetHigh();
		curr_states->SetHigh(l);
		result_states->SetHigh(l);
		result_bit_vector.SetHigh(l);
		PrepareBitVector();
	}
	catch( char * str )
	{
		start=NULL;
		//MessageBox(0,str,"",MB_OK|MB_ICONERROR);
	}
	curr_states_high=-1;
	result_states_high=-1;
}

////----------------------------------------------------------------------------//
//
////--TDFA----------------------------------------------------------------------//

void TDFA::SetExpression(char* s)
{
	try
	{
		nfa.SetExpression(s);
		BuildDFAFromNFA();
	}catch(char* str)
	{
		throw;
	}
}

void TDFA::AddBranch(int source,int dest,char use_min, char use_max)
{
	TBranchType branch_type;
	if (use_min==use_max)  branch_type=BT_CHAR;
	else branch_type=BT_SUBRANGE;
	nodes[source]->branches.Add(TBranch(branch_type,use_min,use_max,nodes[dest]));
	nodes[dest]->branches_in.Add(TBranch(branch_type,use_min,use_max,nodes[source]));
}


bool TDFA::Match(char* s,int &last_char, int first_char)
{
	TFANode* t;
	int s_l=strlen(s);
	if(s_l==-1||start==NULL)return false;
	bool result=start->flag;
	last_char=first_char-1;
	curr_char=first_char;
	t=start;
	while (curr_char<s_l)
	{
		for(int i=0;i<=t->branches.GetHigh();i++)
			switch(t->branches[i].branch_type)
		{
			case BT_SUBRANGE:
				if (IsIn(s[curr_char],t->branches[i].min,t->branches[i].max)) 
				{
					t=t->branches[i].target_node;
					goto end_search;
				}
				break;
			case BT_CHAR:
				if(s[curr_char]==t->branches[i].min)
				{
					t=t->branches[i].target_node;
					goto end_search;
				}
				break;
			default:assert(false);
		}
		return result;
end_search:
		if(t->flag)
		{
			last_char=curr_char;
			result=true;
		}
		curr_char++;
	}
	return t->flag;
}

bool Compare(TArray<TFANode*> &v1,TArray<TFANode*> &v2,int h)
{
	for(int i=0;i<=h;i++)
	{
		for(int k=0;k<=h;k++)if (v1[i]==v2[k])goto end_search;
		return false;end_search:
		int ss;
	}
	return true;
}

void GetStateAlphabet(TFANode* node,TArray<TCharRange> &alphabet)
{
	for(int k=0;k<=node->branches.GetHigh();k++)
		if(node->branches[k].branch_type!=BT_EMPTY)
			GetRanges(alphabet,node->branches[k].branch_type,node->branches[k].min,node->branches[k].max);
}

void TDFA::BuildDFAFromNFA()

{
	TArray<TCharRange> alphabet;
	int curr_min,curr_char;
	bool exist_not_checked,empty_flag;
	//инициализируем первое состояние ДКА и обозначаем его не помеченым
	nfa.curr_states_high=0;
	(*nfa.curr_states)[0]=nfa.start;
	nfa.EClosure();
	nodes.SetHigh(0);
	nodes_nfa_states.SetHigh(0);
	nodes_nfa_states[0]->checked=false;
	nodes_nfa_states[0]->states.SetHigh(nfa.curr_states_high);
	for(int i=0;i<=nfa.curr_states_high;i++)
		nodes_nfa_states[0]->states[i]=(*nfa.curr_states)[i];
	//
	exist_not_checked=true;
	while(exist_not_checked)
	{
		exist_not_checked=false;
		for(int i=0;i<=nodes.GetHigh();i++)
			if (!nodes_nfa_states[i]->checked) 
			{
				exist_not_checked=true;
				nodes_nfa_states[i]->checked=true;
				//Формируем алфавит для данного состояния ДКА
				alphabet.SetCount(0);
				for(int t=0;t<=nodes_nfa_states[i]->states.GetHigh();t++)
					GetStateAlphabet(nodes_nfa_states[i]->states[t],alphabet);
				//Для каждого символа алфавита получаем новое множество состояний НКА
				curr_min=-1;
				while(curr_min<255)
				{
					//
					curr_char=256;
					//Определяем наименьший диапазон содержащий одинаковые символы
					for(int t=0;t<=alphabet.GetHigh();t++){
						if      (alphabet[t].min>curr_min  && alphabet[t].min<=curr_char)  curr_char=alphabet[t].min-1;
						else if (alphabet[t].max>=curr_min && alphabet[t].max<curr_char )  curr_char=alphabet[t].max;
					}
					//Определяем является ли этот диапазон не пустым
					empty_flag=true;
					for(int t=0;t<=alphabet.GetHigh();t++){
						if (curr_char>=alphabet[t].min && curr_char<=alphabet[t].max)
						{
							empty_flag=false;
							break;
						}
					}
					if (curr_char==256)  break;
					if (curr_min==-1 || empty_flag) 
					{
						curr_min=curr_char+1;
						continue;
					}
					if (empty_flag)  continue;
					//Получаем новое множество состояний НКА
					nfa.curr_states_high=nodes_nfa_states[i]->states.GetHigh();
					for(int t=0;t<=nfa.curr_states_high;t++)
						(*nfa.curr_states)[t]=nodes_nfa_states[i]->states[t];
					if (!nfa.Move(curr_char)) assert(false);//ошибка в НКА
					nfa.EClosure(true);
					int n=-1;
					//Определяем, имеется ли новое множество состояний в nodes_nfa_states
					for(int k=0;k<=nodes.GetHigh();k++)
					{
						int h=nodes_nfa_states[k]->states.GetHigh();
						if(h==nfa.curr_states_high && Compare(nodes_nfa_states[k]->states,*nfa.curr_states,h))
						{
							n=k;
							break;
						}
					}
					//Если новое множество состояний раньше не обрабатывалось, то добаляем как необработанное
					if(n==-1)
					{
						nodes.Add();
						n=nodes.GetHigh();
						nodes_nfa_states.SetHigh(nodes.GetHigh());
						{
							nodes_nfa_states[nodes.GetHigh()]->checked=false;
							nodes_nfa_states[nodes.GetHigh()]->states.SetHigh(nfa.curr_states_high);
							for(int t=0;t<=nfa.curr_states_high;t++)
								nodes_nfa_states[nodes.GetHigh()]->states[t]=(*nfa.curr_states)[t];
						}
					}
					AddBranch(i,n,char(curr_min),char(curr_char));
					//
					curr_min=curr_char+1;
				}
			}
	}
	//
	start=NULL;
	for (int i=0;i<=nodes.GetHigh();i++) 
	{
		int h=nodes_nfa_states[i]->states.GetHigh();
		if(start==NULL)
		{
			for (int k=0;k<=h;k++)if (nodes_nfa_states[i]->states[k]==nfa.start)
			{
				start=nodes[i];
				break;
			}
		}
		for (int k=0;k<=h;k++)if (nodes_nfa_states[i]->states[k]==nfa.finish)
		{
			nodes[i]->flag=true;
			break;
		}
	}
}