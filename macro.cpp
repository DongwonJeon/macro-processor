/*
2020111983 ������ ��ũ�� ���μ��� ���� ����
*/
#define _CRT_SECURE_NO_WARNINGS
#include "table.cpp"
#define MAXSIZE 10
#define MAXLINE 100
#define MAXARG 30

using namespace std;

int hashing(string s);
void insertnam(string nam);
void insertindex(string nam, int s, int e);
bool findnam(string nam, int& s, int& e);
void setargtab(string s);
void mgetline(int &index);
void processline();
void define();
void expand(int s, int e);
void setargtab(string s);
void setargvalue(string s);
void uniqueval(int s, int e, int macronum, int level);
void substitute(string& s);
void setval(string s1, string s2);
bool condition(string s);

int index = 0; // �д� �� �� 
bool expanding = false; // Ȯ�� ������
bool set = false; // set ��������
ifstream inpf;
ofstream outpf;
string line; // ����� ����
string element[MAXARG]; // keyword ����� ����
string replacement[MAXARG]; // keyword ����� ����
string label[MAXLINE];
string opcode[MAXLINE];
string operand[MAXLINE];
string backup[MAXLINE]; // operand�� unique label�� ��ü�ϱ� ���� ����

NAMTAB* namta[MAXSIZE];
DEFTAB defta;
ARGTAB argta[MAXARG];

int backupindex = 0; // backup������ index
int defindex = 0; // deftab�� �ֱ� ���� index
int argindex = 0; // argtab�� �ִ� index
int argend=0; // keyword ����� ����
int macronum = 0; // ���� �� ��° macro����(nested X)

int main(int argc, char** argv) {

	if (argc == 2) {
		inpf.open(argv[1]);
	}
	else inpf.open("macro");
	if (inpf.fail()) {
		cout << "File Error!" << endl;
		exit(100);
	}
	outpf.open("output"); // �������
	while (opcode[index] != "end") {
		mgetline(index);
		processline();
	}
	inpf.close();
	outpf.close();
}

int hashing(string s) {//�ؽð��� ��� �Լ�
	int hash = 0;
	const char* k = s.c_str();//string���� char�� ��ȯ�Ͽ� �ؽð��� ��´�
	while (*k != NULL) {
		hash = hash + (int)(*k);
		k++;
	}
	return hash % MAXSIZE;
}

void insertnam(string nam) { // namtab�� name���� �ִ� �Լ�
	int index = hashing(nam);
	NAMTAB* newtab = new NAMTAB;
	newtab->setname(nam);
	if (namta[index] == NULL) {
		namta[index] = newtab;
	}
	else {
		NAMTAB* cursor = namta[index];
		while (cursor->next != NULL) {
			cursor = cursor->next;
		}
		cursor->next = newtab;
	}
}

void insertindex(string nam, int s, int e) { // namtab�� name�� �����ؼ� startindex�� endindex ����
	int index = hashing(nam);
	if (namta[index]->name == nam) {
		namta[index]->setindex(s, e);
	}
	else {
		NAMTAB* cursor = namta[index];
		while (cursor->next != NULL) {
			cursor = cursor->next;
			if (cursor->name == nam)
				cursor->setindex(s, e);
		}
	}
}

bool findnam(string nam, int& s, int& e) {//NAMTAB�� �����ϴ��� Ȯ��
	int index = hashing(nam);//�ؽð��� ���ϰ�
	NAMTAB* cursor = namta[index];
	while (cursor != NULL) {
		if (cursor->name == nam) {//name ���� �� true ��ȯ
			s = cursor->startindex;
			e = cursor->endindex;
			return true;
		}
		cursor = cursor->next;// �ٸ� ��� ���� �ּҷ� �̵�
	}
	return false; //���� ��� false ��ȯ
}

void setargtab(string s) { // argtab�� argument�� �ִ� �Լ�
	istringstream iss(s);
	int i = 0;
	string temp1;
	string temp2;
	if (s.find("=") == string::npos) { // position ����� ���
		while (getline(iss, element[i], ',')) {
			argta[argindex].setarg(element[i++]);
			argindex++;
		}
	}
	else { // keyword ����� ���
		while (getline(iss, element[i], ',')) {
			temp1 = "";
			temp2 = "";
			temp1 = element[i].substr(0, element[i].find("="));
			if (element[i].back() != '=') { // = �ڿ� �μ��� �ִ� ��� value�� ����
				temp2 = element[i].substr(element[i].find("=") + 1);
				argta[argindex].setarg(temp1);
				argta[argindex].setvalue(temp2);
				i++;
				argindex++;
			}
			else { // = �� ���� ��� argument�� set
				argta[argindex].setarg(temp1);
				argta[argindex].setvalue(" ");
				i++;
				argindex++;
			}
		}
	}
	argend = argindex;
}

void setargvalue(string s) { // argtab�� value�� �ִ� �Լ�
	istringstream iss(s);
	int i=0, j=0;
	string temp;
	string temp1;
	string temp2;
	if (s.find("=") == string::npos) { // position ����� ���
		for (int a = 0; a < MAXARG; a++) {
			if (argta[a].is_val_empty()) {
				j = a;
				break;
			}
		}
		while (getline(iss, replacement[i], ',')) {
			if (replacement[i].find("(") != string::npos) { // �迭�� �Է¹޴� ��� ����ó��
				while (1) {
					temp = "";
					getline(iss, temp, ',');
					replacement[i].append(temp);
					if (temp.find(")") != string::npos)
						break;
				}
			}
			argta[j].setvalue(replacement[i++]);
			j++;
		}
	}
	else {
		while (getline(iss, replacement[i], ',')) {
			temp1 = replacement[i].substr(0, replacement[i].find("=")); // = �� �������� ������ ����
			temp2 = "";
			if (replacement[i].back() != '=') {
				temp2= replacement[i].substr(replacement[i].find("=") + 1);
				for (int a = 0; a < MAXARG; a++) {
					if (argta[a].argument.find(temp1) != string::npos) { // temp1�� argument���� ã���� value�� temp2 set
						argta[a].setvalue(temp2);
					}
				}
			}
			else {
				temp2 = " ";
				for (int a = 0; a < MAXARG; a++) {
					if (argta[a].argument.find(temp1) != string::npos) { // temp1�� argument���� ã���� value�� temp2 set
						argta[a].setvalue(temp2);
					}
				}
			}
			i++;
		}
	} // keyword ����� ���
	for (int i = 0; i < argend; i++) { // �Է¹��� �ʾ��� ��� ���� ó��
		if (argta[i].is_val_empty())
			argta[i].setvalue(" ");
	}
}
// keyword �� position�� ���� ���� �������� �ʾ���

void uniqueval(int s, int e, int macronum, int level) { // operand �κ��� unique value�� �ٲٴ� �Լ�
	string temp="&&";
	temp[0] = (char)(96 + macronum); // unique label�� ����
	temp[1] = (char)(96 + level);
	for (int i = s; i < e; i++) {
		for (int j = 0; j < backupindex; j++) {
			if (defta.moperand[i] == backup[j]) {
				if (defta.moperand[i][0] == '$')
					defta.moperand[i].insert(1, temp);
				else
					defta.moperand[i].insert(0, temp);
			}
		}
	}
}

void substitute(string &s) { // argument�� vaule�� �ٲٴ� �Լ�
	string::size_type a;
	for (int i = 0; i < argend; i++) {
		string temp = argta[i].argument;
		if (s.find(temp) != string::npos) {
			if(s.substr(s.find(temp)+temp.length(), 1)=="'"|| s.substr(s.find(temp) + temp.length(), 1) == ","|| \
				s.substr(s.find(temp) + temp.length(), 1) == " "|| s.substr(s.find(temp) + temp.length(), 1).empty())
				s.replace(s.find(temp), temp.length(), argta[i].value);
			if (s.find("->") != string::npos) {
				a = s.find("->");
				s.erase(a, 2); // -> ����
			}
		}
	}
}

void setval(string s1, string s2) { // set �� �� value ���� ���ڳ� ������ �޴´ٰ� ����
	for (int i = 0; i < argend; i++) {
		if (argta[i].argument == s1) {
			argta[i].setvalue(s2);
		}
	}
}

bool condition(string s) { // if ���̳� while�� ��
	stringstream ss(s);
	string arg1;
	string cond;
	string arg2;
	string temp;
	ss >> arg1;
	ss >> cond;
	ss >> arg2;
	ss >> temp;
	if (arg1.front() == '(') {
		arg1.erase(0, 1);
	}
	if (arg1.empty()) arg1 = " ";
	if (!temp.empty()) arg2.append(temp);
	if (arg2 == "'')") {
		arg2 = " ";
	}
	else if (arg2 == ")") {
		arg2 = " ";
	}
	if (arg2.back() == ')') {
		arg2.pop_back();
	}
	if (cond == "lt") { // le�� ge�� �������� �ʰ� lt�� gt�� ������ ���� �۵��Ѵ�
		if (stoi(arg1) < stoi(arg2)) return true;
		else return false;
	}
	else if (cond == "eq") {
		if (arg1 == arg2) return true;
		else return false;
	}
	else if (cond == "ne") {
		if (arg1 != arg2) return true;
		else return false;
	}
	else if (cond == "gt") {
		if (stoi(arg1) > stoi(arg2)) return true;
		else return false;
	}
	else {
		outpf << "Condition Error" << endl;
		return false;
	}
}

void mgetline(int &index) { // getline
	string s;
	string temp = "";
	string tlabel;
	string topcode;
	string toperand;
	if (expanding) { // Ȯ�� ���� ��� deftab���� �о��
		line = "";
		tlabel = defta.mlabel[index];
		topcode = defta.mopcode[index];
		toperand = defta.moperand[index];
		substitute(toperand); // ��ü
		if (topcode == "set") { // set�� ��� value set
			set = true;
			setval(tlabel, toperand);
		}
		if (topcode == "if") { //if���� ���
			if (condition(toperand)) { // ������ ��������
				index++;
				while (defta.mopcode[index] != "else" && defta.mopcode[index] != "endif") { // else�� endif����
					mgetline(index);
					processline();
					index++;
				}
				if (defta.mopcode[index] == "else") { // ������ �¾����Ƿ� else���� �۵����� �ʴ´�
					while (defta.mopcode[index] != "endif") {
						index++;
					}
				}
			}
			else { 
				while (defta.mopcode[index]!="else"&&defta.mopcode[index] != "endif") {
					index++;
				}
				if (defta.mopcode[index] == "else") { // Ʋ�� ��� else���� ����
					index++;
					while (defta.mopcode[index] != "endif") {
						mgetline(index);
						processline();
						index++;
					}
				}
			}
			set = true; // if�� ��� ����ó��
		}
		/*
		LOOP�� �����Ͽ����� �迭�� ��Ģ������ �̱��� �����̱� ������
		���� ������ �������� �ʴ´�.
		*/
		else if (topcode == "while") { 
			int b_index = index;
			if (condition(toperand)) { // ������ �̸� ��������
				while (condition(toperand)) { // �ݺ�
					index++;
					while (defta.mopcode[index] != "endw") { // while���� ������ ������ ��� �ݺ�
						mgetline(index);
						processline();
						index++;
					}
					index = b_index; // while�� ���� index
				}
				
			}
			else { // �ƴ� ��� while�� �ѱ�� ���� �ٳѱ�
				while (defta.mopcode[index] != "endw") {
					index++;
				}
			}
			set = true; // while�� ��� ����ó��
		}
		line = tlabel + " " +  topcode + " " + toperand; // processline �Լ����� ����ϱ� ���� line�� append
	}
	else {
		getline(inpf, line);//�� �پ� �о��
		stringstream ss(line);//�ɰ��� ���� stringstream �̿�
		s = line.front();//label�� ���� ���� �����ϱ� ����
		if (s.compare(" ") == 0) {//���� ���ٸ�
			label[index] = s;//���� ����
			ss >> opcode[index];
			ss >> operand[index];
			ss >> temp;
			while (!temp.empty()||temp!="") {
				operand[index].append(" ");
				operand[index].append(temp); // �޸� �ڿ� ���Ⱑ ���� ��� ó��
				temp = "";
				ss >> temp;
			}
		}
		else {//���� �ִٸ� ���������� ����
			ss >> label[index];
			ss >> opcode[index];
			ss >> operand[index];
			ss >> temp;
			while (!temp.empty()||temp!="") {
				operand[index].append(temp); // �޸� �ڿ� ���Ⱑ ���� ��� ó��
				operand[index].append(" ");
				temp = "";
				ss >> temp;
			}
		}
	}
}

void processline() { // Ȯ��, ���Ƿ� �ѱ�ų� ����ϴ� �Լ�
	int s, e;
	if (findnam(opcode[index], s, e)) { // macro �̸��� ��ġ�� ��� expand
		expand(s, e);
	}
	else if (opcode[index] == "macro") { // opcode==macro�� ��� ����
		define();
	}
	else { // ���
		if (!set) {
			outpf << line << endl;
			cout << line << endl;
		}
		set = false;
	}
}

void define() { // ��ũ�� ���� �Լ�
	int startindex[MAXSIZE] = { 0 };
	int endindex[MAXSIZE] = { 0 };
	int level = 1;
	macronum++;
	startindex[level] = index;
	insertnam(label[index]);
	defta.setline(line, label[index], opcode[index], operand[index], defindex);
	defindex++;
	setargtab(operand[index]); // argument ����
	while (level > 0) {
		index++;
		mgetline(index);
		//if not comment line (���� X)
		if (opcode[index] != "set") {
			string temp = "&&";
			temp[0] = (char)(96 + macronum); // unique label�� ����
			temp[1] = (char)(96 + level);
			if (!label[index].empty() && label[index] != " ") {
				backup[backupindex++] = label[index];
				if (label[index][0] == '$')
					label[index].insert(1, temp);
				else
					label[index].insert(0, temp);
			}
		}
		defta.setline(line, label[index], opcode[index], operand[index], defindex);
		defindex++;
		if (opcode[index] == "set") { // set�� ��� argtab�� �ִ´�
			argta[argindex].setarg(label[index]);
			argindex++;
			argend++;
		}
		if (opcode[index] == "macro") {
			level++;
			startindex[level] = index;
		}
		else if (opcode[index] == "mend") {
			endindex[level] = index;
			insertindex(label[startindex[level]], startindex[level], endindex[level]); // namtab�� index�� ����
			uniqueval(startindex[level], endindex[level], macronum, level); // unique label�� operand �κе� ó��
			index++;
			level--;
		}
	}
}

void expand(int s, int e) { // Ȯ�� �Լ�
	expanding = true; // Ȯ�� ����
	setargvalue(operand[index]); // value���� ����
	string temp;
	temp[0] = (char)(65);
	string back = label[index]; // ������ label ����
	if (!label[index].empty() && label[index] != " ") // ������� ���� ��� unique label
		label[index].insert(0, ".");
	outpf << label[index] << " " << defta.mlabel[s] << " " << operand[index] << endl;
	if (!label[index].empty() && label[index] != " ")
		outpf << back;
	cout<< label[index] << " " << defta.mlabel[s] << " " << operand[index] << endl;
	if (!label[index].empty() && label[index] != " ")
		cout << back;
	index++;
	for( int i=s+1; i<e; i++){ // ����ΰ� �ƴ� ������ line�� ���
		mgetline(i);
		processline();
	}
	expanding = false;
}
