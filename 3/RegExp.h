#pragma once

#include "baluLib.h"

//����������� �������:
//  '?', '+', '*' - �������������� (���� ��� ����, ���� � �����, ����� ����� ��� ��������������)
//  '(', ')' - ������������ ������
//  '|' - �������� ���
//  '[', ']' - ������������
//  '{', '}' - ������������ (s - ���������� ������� ������ � �����, r - ���������������������)
//  '#' - ���� ������� �� ���� (#20, #120 � �.�.)
//  '.' - ����� ������
//  '\' - ������ �������� � ����� ����������� ��������
//  ��� ����� ������ ������������ ������� ����� ��� ��������� '\',
//��� ������� ����� ����������� ������������ ���� ����
//������ ��������:
//������������:

//DONE: ������� ��� �������������� ��������
//DONE: ���������� ��������� #13#10 ��� ���� �������� ��� � ������
//TODO: �������������� ��������� ��� ������� ����

enum TBranchType
{
	BT_NOT_INITIALIZED,
	BT_EMPTY,    //E branch
	BT_CHAR,     // wqwgnbd
	BT_SPACE,    // \s
	BT_WORD,     // \w
	BT_DIGIT,    // \d
	BT_NEW_LINE, // \n
	BT_TAB,       // \t
	BT_SUBRANGE, // a-z
	BT_NOT_CHAR,   // [^zqw]
	BT_NOT_SPACE,  // \S
	BT_NOT_WORD,   // \W
	BT_NOT_DIGIT,  // \D
	BT_NOT_NEW_LINE, // \N
	BT_NOT_TAB,       // \T
	BT_ANY_CHAR    // .
};


struct TCharRange
{
	char min;
	char max;
	TCharRange():min(0),max(0){}
	TCharRange (char use_min, char use_max):min(use_min),max(use_max){}
	TCharRange (char use_min_max):min(use_min_max),max(use_min_max){}
};

//��������� ��������
struct TFANode;
struct TBranch
{
	TBranchType branch_type;
	char min,max;
	TFANode* target_node;
	TBranch():branch_type(BT_NOT_INITIALIZED),target_node(NULL){}
	TBranch(TBranchType use_branch_type,char use_min,char use_max,TFANode* use_target_node)
		:branch_type(use_branch_type),min(use_min),max(use_max),target_node(use_target_node){}
};

struct TFANode
{
	TArray<TBranch> branches_in,branches;
	int index;
	bool flag;
	TFANode():flag(false),index(-1){}
};

//������������������� �������� ������� (finite automoton)

class TNFA
{
private:
	char* expr;
	int expr_length;
	int curr_char;
	//��������� ������������������ ��������
	TPointerArray<TFANode> nodes;
	TFANode *start,*finish;
	//������� ������� ����������� ������
	int parenthnes_level;
	//������� ������ � ��� ������� ��� ������������� ������ �������� (Move, EClosure)
	TArray<bool> result_bit_vector;
	TArray<TFANode*> *curr_states,*result_states;
	int curr_states_high,result_states_high;
	//
	void AddBranch(TFANode* node,TFANode* use_target,TBranchType use_branch_type=BT_EMPTY,char use_min=0,char use_max=0);
	void ReplaceBranch(TArray<TBranch> &branches,TFANode* branch,TBranch new_val);
	void ReplaceBranch(TArray<TBranch> &branches,TFANode* branch,TFANode* new_val);
	//
	void PrepareBitVector();
	void BuildAlternatives(TFANode* start,TFANode* finish);
	void AddAlternativeRegister(TFANode* start,TFANode* finish);
	void AddNodeAlternativeRegister(TFANode* node,TFANode* finish);
	void BuildNFA(TFANode* start,TFANode* finish,int level);
	void EClosure(bool except_unimportant_nodes=false);
	bool Move(char a);
	void GetTerm(bool alternatives_mode,TBranchType &btype,char &val);
	void Optimize();
	void Clean();
public:
	void SetExpression(char* s);
	bool Match(char* s);
	TNFA()
	{
		start=NULL;
		curr_states=new TArray<TFANode*>;
		result_states=new TArray<TFANode*>;
	}
	~TNFA()
	{
		delete curr_states;
		delete result_states;
	}
	friend class TDFA;
};

//����������������� �������� �������

class TDFA
{
	struct Tnodes_nfa_state
	{
		TArray<TFANode*> states;
		bool checked;
	};
private:
	TNFA nfa;
	int curr_char;
	//��������� ������������������ ��������
	TPointerArray<TFANode> nodes;
	//��������� ���������� ������� ��������� ��� ��� ���������� ���
	TPointerArray<Tnodes_nfa_state> nodes_nfa_states;    
	//
	TFANode* start;
	//
	void BuildDFAFromNFA();
	void Clean();
	void AddBranch(int source,int dest,char use_min, char use_max);
public:
	void SetExpression(char* s);
	bool Match(char* s,int &last_char, int first_char=0);
};