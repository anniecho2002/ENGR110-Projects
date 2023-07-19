#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
using namespace std;

vector<string> code;
string state = "s0";
string output = "";

int SaveCodeFile(std::string fileName){
  std::ofstream codeOutFile( fileName );
  // write whole code to the file
   for( unsigned int i = 0 ; i < code.size(); i++ ) {
      //cout<<code.at(i)<<endl;
      codeOutFile<<code.at(i)<<std::endl;
    } 
    codeOutFile.close();
    return 0;
}

// returns whole specification file as one string
// whitespaces and line ending characters are removed
std::string ReadSpecFile(std::string fileName){
  std::cout<<"Reading specifications file "<<fileName<<std::endl;
  std::string out;
  std::ifstream inputFile( fileName );
  std::string line;
  out = "";
  std::string n1;
  while(getline(inputFile,line)) { 
   //std::cout<<"from file="<<line<<std::endl;
   // remove all new line characters;
   line.erase( std::remove(line.begin(), line.end(),'\r'), line.end() );
   line.erase( std::remove(line.begin(), line.end(),'\n'), line.end() );
   line.erase( std::remove(line.begin(), line.end(),' '), line.end() );
   out.append(line);
  }
  //cout<<" input="<< input<<endl;
  return out;
}

void add_start(){
	vector<string> start = {"#include <iostream>", "using namespace std;", "string state;", "string input;", "int accept = 0;", "int Proc(string input)", "{"};
	for (unsigned int i=0; i<start.size(); i++){
		code.push_back(start[i]);
	}
}

void add_end(){
	vector<string> end = {"while(1){", "cout<<\" state=\"<<state<<\" enter signal\"<<endl;", "cin>>input;", "accept = Proc(input);", "cout<<\" new state=\"<<state<<endl; // p", "cout<<\"accepting: \"<<accept<<endl;", "}", "}"};
	for (unsigned int i=0; i<end.size(); i++){
		code.push_back(end[i]);
	}
}

struct Transition{
  std::string input_signal;
  std::string dest_state;
};

struct State{
  std::string name;
  std::vector<Transition> transitions;
};

struct FSM_out{
   std::vector<State> states;
   std::string accepting_name;
   std::string initial_name;
   State state_in_progress;
};

FSM_out fsm;
Transition transition_in_progress;


void processChar(char in_char){
	if (state == "s0"){ // beginning state, checks for i, a, or s
		if 		(in_char == 'i'){state = "initial";}
		else if (in_char == 'a'){state = "accepting";}
		else if (in_char == 's'){state = "state";}
	}
	
	
	else if (state == "initial"){ // initial name at the start
		if (in_char == '{'){state = "initial2";}
		else {state = "error";} 
	}
	else if (state == "initial2"){
		if (in_char != '}'){output = output + in_char;}
		else {
			fsm.initial_name = output;
			output.clear();
			state = "s0";
		}
	}
	
	
	// accepting name at the start
	else if (state == "accepting"){
		if (in_char == '{'){state = "accepting2";}
		else {state = "error";}
	}
	else if (state == "accepting2"){
		if (in_char != '}'){output = output + in_char;} 
		else { 
			fsm.accepting_name = output;
			output.clear();
			state = "s0";
		} 
	}
	
	// state has been declared/needs to be opened, checks for '{'
	else if (state == "state"){  
		if (in_char == '{'){state = "state2";}
		else {state = "error";}
	}
	// state has been opened, checks for n, t, or '}'
	else if (state == "state2"){
		if 		(in_char == 'n'){state = "n";}
		else if (in_char == 't'){state = "t";}
		else if (in_char == '}'){
			fsm.states.push_back(fsm.state_in_progress); // add the SIP to the vector
			fsm.state_in_progress = State(); // hopefully clears the SIP
			state = "s0"; // go back to the start
		}
		else {state = "error";}
	}
	
	
	// inside the state, we get an n, checks for '{'
	else if (state == "n"){
		if (in_char == '{'){state = "n2";}
		else {state = "error";}
	}
	// we have received '{' and records in_char until done
	else if (state == "n2"){
		if (in_char != '}'){output = output + in_char;} 
		else {
			fsm.state_in_progress.name = output;
			output.clear();
			state = "state2";
		}
	}
	
	
	// inside the state, we get a t
	else if (state == "t"){
		if (in_char == '{'){state = "t2";}
		else {state = "error";}
	}
	// t has been opened
	else if (state == "t2"){
		if (in_char == 'i'){state = "i";}
		else if (in_char == 'd'){state = "d";}
		else {state = "error";} // if we don't get an i or d
	}
	// t needs to be closed, comes here from when dtransition_in_progress; is closed
	else if (state == "t3"){
		if (in_char == '}'){ // transition has been closed
			fsm.state_in_progress.transitions.push_back(transition_in_progress); // adds the TIP to the SIP transitions vector
			transition_in_progress = Transition(); // should hopefully clear the transition in progress
			state = "state2";
		}
		else {state = "error";}
	}
	
	
	// inside an i bracket
	else if (state == "i"){
		if (in_char == '{'){state = "i1";}
		else {state = "error";}
	}
	else if (state == "i1"){
		if (in_char != '}'){output = output + in_char;}
		else { // i has been closed
			transition_in_progress.input_signal = output;
			output.clear();
			state = "t2";
		}
	}
	
	
	// inside a d bracket
	else if (state == "d"){
		if (in_char == '{'){state = "d1";}
		else {state = "error";}
	}
	else if (state == "d1"){
		if (in_char != '}'){output = output + in_char;}
		else { // d has been closed
			transition_in_progress.dest_state = output; // adds destination to the TIP
			output.clear();
			state = "t3";
		}
	}	
}


// Changes the FSM structure into code
void toCode(){
	add_start();
	for(unsigned int i = 0; i < fsm.states.size(); i++){
		if (i == 0){ code.push_back("if (state == \"" + fsm.states[i].name + "\"){"); }
		else { code.push_back("else if (state == \"" + fsm.states[i].name + "\"){");}

		for(unsigned int j = 0; j < fsm.states[i].transitions.size(); j++){
			if(j == 0){
				code.push_back("if (input == \"" + fsm.states[i].transitions[j].input_signal + "\"){");
			}
			else{
				code.push_back("else if (input == \"" + fsm.states[i].transitions[j].input_signal + "\"){");
			}
			code.push_back("state = \"" + fsm.states[i].transitions[j].dest_state + "\";");

			if(fsm.states[i].transitions[j].dest_state == fsm.accepting_name){
				code.push_back("return 1;");
			}
			else{
				code.push_back("return 0;");
			}
			code.push_back("}");
		}
		code.push_back("}");
	}
	code.push_back("}");
	code.push_back("int main(){");
	code.push_back("state = \"" + fsm.initial_name + "\";");
	add_end(); 
}


int deadEnd(){
	int deadEndFound = 1;
	for (unsigned int i = 0; i<fsm.states.size(); i++){
		for (unsigned int j = 0; j<fsm.states[i].transitions.size(); j++){
			int val = 0;
			for (unsigned int z = 0; z<fsm.states.size(); z++){
				if (fsm.states[i].transitions[j].dest_state == fsm.states[z].name){
					val = 1;
				}
			}
			if (val == false){
				deadEndFound = val;
			}
		}
		
		
	}
	return deadEndFound;
}


int main(){ 
	string fileName = "cat_err.txt";
    //cout<<"Enter filename: ";
    //cin>>fileName;
	string fileText = ReadSpecFile(fileName);
	
	for (unsigned int i=0; i<fileText.length(); i++) {processChar(fileText[i]);}
	
	// after running through the entire text file, if it has encountered an error
	if (state == "error" || deadEnd() == 0){
		if(state == "error"){cout<<"Malformed specification file. Error."<< endl;}
		if (deadEnd() == 0){cout<<"Dead end found."<< endl;}
	}
	else{
		toCode();
		SaveCodeFile("code.cpp");
	}
}
