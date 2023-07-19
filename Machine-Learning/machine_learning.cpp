// single neuron code
// all functions for Assignment 1
// and Ex1 of Assignment 2 implemented

#include <iostream>
#include <fstream> // for logging
#include <math.h>
#include <vector>
#include <numeric>
#include <algorithm>
#include <vector>

//activation function
double sigmoid(double x){
    return 1.0 / (1.0 + exp(-x));
}

// one point/sample of training set. 
struct train_point_t{
	std::vector<double> inputs;
	double target;
};

//vector of training samples
struct train_set_t{
  std::vector<train_point_t> train_points;	
};

// usually loaded from file, but in this case training set is small
void set_train_set(train_set_t& ts){
	train_point_t p1;
	p1.inputs = {0.0, 0.0}; //initialize vector
	p1.target = 0.0;
	ts.train_points.push_back(p1);
	train_point_t p2;
	p2.inputs = {0.0, 1.0};
	p2.target = 0.0;
	ts.train_points.push_back(p2);
	train_point_t p3;
	p3.inputs = {1.0, 0.0};
	p3.target = 0.0;
	ts.train_points.push_back(p3);
	train_point_t p4;
	p4.inputs = {1.0, 1.0};
	p4.target = 1.0;
	ts.train_points.push_back(p4);
	
}

// prints all training saples/points
void print_set(const train_set_t& ts){
	for(train_point_t tp:ts.train_points){
		std::cout<<" Inputs ";
		for(double in:tp.inputs) std::cout<<in<<" ";
		std::cout <<" Output: "<<tp.target<<" ";
		std::cout<<std::endl;
	}
}

// to help with plotting the search convergence
// vector to store 
std::vector<double> convergence;
void save_vector_to_file(std::vector<double> v){
	std::string file_name;
	std::cout<<" Enter file name for convergence graph:";
	std::cin>>file_name;
	std::ofstream ofs;
    std::cout<<"Saving vector "<<file_name<<" ..."<<std::endl;
    ofs.open(file_name.c_str(), std::ofstream::out);//, std::ofstream::out | std::ofstream::trunc);
	if (ofs.is_open()){
		int count =0 ;
		for(double ve:v){
			ofs<<ve<<std::endl;
			count++;
		}
		ofs.close();
	}
}


struct Neuron{
	std::vector<double> weights;   // weights
	double bias;
	double z; // before sigmoid
	double y; // outputs
	void init(int nw);
	double forward(std::vector<double> inputs);
	void print_neuron();
};

// prototype
void draw_output(Neuron& n);

// sets weights and biases
void Neuron::init(int n){
	
	bias = 0.0;
	weights.reserve(2); //reserve memory for weights
	weights.emplace_back(0.0); // faster compared with "push_back()"
	weights.emplace_back(0.0);
	std::cout<<" weight size="<<weights.size();
}

// calculates output
double Neuron::forward(std::vector<double> inputs){
	z = bias;
	for (uint i = 0 ; i < weights.size() ; i++){
		z = z + weights[i]*inputs[i];
	}
	// shorter version
//	z = std::inner_product(weights, weights + nw,inputs.begin(),bias);
	y =  sigmoid(z);
	return y;
}


void Neuron::print_neuron(){
    std::cout<<" bias="<<bias;
    std::cout<<" w0="<<weights[0];
    std::cout<<" w1="<<weights[1]<<std::endl;
	
}

// what is the difference now between 
// output and target
double error(Neuron& neuron, double t){
	return (neuron.y -t);
}

// combined squared error for all training samples
double total_error(Neuron& neuron, const train_set_t& ts){
    double tot_error = 0.0;
    for ( uint i =0 ; i < ts.train_points.size() ; i++){
		double output = neuron.forward(ts.train_points[i].inputs);
		neuron.y = output;
		double err = error(neuron, ts.train_points[i].target);
		tot_error = tot_error + (err*err);
	 }
     return tot_error;
}

// enter 
void manual_entry(Neuron& n,train_set_t ts){
	std::cout<<"Enter bias: ";
	std::cin>>n.bias;
	std::cout<<"Enter Weight 0: ";
	std::cin>>n.weights[0];
	std::cout<<"Enter Weight 1: ";
	std::cin>>n.weights[1];
	double total_error = 0.0;
	
	for (int i=0; i<4; i++){
		double output = n.forward(ts.train_points[i].inputs);
		n.y = output;
		double err = error(n, ts.train_points[i].target);
		total_error = total_error + (err*err);
	}
	std::cout<<"Total error: "<<total_error;
	
}

void global_search(Neuron& neuron,const train_set_t& train_set ){
    double min_tot_err = 10000.0;
    double w0_min_err=0.0,w1_min_err=0.0,bias_min_err=0.0;
	for (double bias = -10.0; bias < 10.0; bias = bias + 0.1){
	  for ( double w0 = -10.0; w0 < 10.0; w0 = w0 + 0.1){
	   for ( double w1 = -10.0; w1 < 10.0; w1 = w1 + 0.1){
 	      //neuron.set_neuron({w0,w1}, bias);
 	      neuron.bias = bias;
 	      neuron.weights[0] = w0;
 	      neuron.weights[1] = w1;
 	      double total_error = 0.0;
 	      for (int i=0; i<4; i++){
			  double output = neuron.forward(train_set.train_points[i].inputs);
			  neuron.y = output;
			  double err = error(neuron, train_set.train_points[i].target);
			  total_error = total_error + (err*err);
		  }
		  if (total_error < min_tot_err){
			  min_tot_err = total_error;
			  w0_min_err = w0;
			  w1_min_err = w1;
			  bias_min_err = bias;
		  }

 	      
	    }
       }
	}
    
    std::cout<<"Min Error: "<<min_tot_err<<std::endl;
    std::cout<<"W0 Min Error: "<<w0_min_err<<std::endl;
    std::cout<<"W1 Min Error: "<<w1_min_err<<std::endl;
    std::cout<<"Bias Min Error: "<<bias_min_err<<std::endl;

}

void gradient_search(Neuron& neuron,const train_set_t& train_set){
    double d = 0.01;
    double db, dw0,dw1;
    double learn_rate = 30.0;
    double final_error = 0;
    int n_step = 0;
    int min_step = 0; // the number of steps needed to find final_error
    
    while (n_step<300){
		double total_error_1 = 0;
		double total_error_2 = 0;
		
		for (uint i = 0 ; i < train_set.train_points.size(); i++){
			total_error_1 = total_error(neuron, train_set);
			
			neuron.bias += d;
			total_error_2 = total_error(neuron, train_set);
			db = (total_error_2 - total_error_1)/d; // finds the diff 
			neuron.bias -= d;
			
			neuron.weights[0] += d;
			total_error_2 = total_error(neuron, train_set);
			dw0 = (total_error_2 - total_error_1)/d;
			neuron.weights[0] -= d;
			
			neuron.weights[1] += d;
			total_error_2 = total_error(neuron, train_set);
			dw1 = (total_error_2 - total_error_1)/d;
			neuron.weights[1] -= d;
			
			neuron.bias -= learn_rate * db;
			neuron.weights[0] -= learn_rate * dw0;
			neuron.weights[1] -= learn_rate * dw1;
			
			total_error_1 = total_error(neuron, train_set);
			
			convergence.push_back(total_error_1);
			final_error = total_error_1;
			
			if (final_error < 0.01 && min_step == 0){
				min_step = n_step;
				// finds the minimum step number (when it goes below 0.01
			}			
			n_step++;                   
		}
    }
    
    for ( uint i = 0 ; i < train_set.train_points.size() ; i++){
	     neuron.forward(train_set.train_points.at(i).inputs);
         double e = train_set.train_points.at(i).target - neuron.y;
         std::cout<<" i= "<<i<<" y="<<neuron.y<<" e="<<e<<std::endl;

    }
    neuron.print_neuron();
    std::cout<<"Final error: "<<final_error<<std::endl;
    std::cout<<"Min Step Number: "<<min_step<<std::endl;

}

// draws y(x0,x1) 	      double output = n.forward(train_set.train_points[i].inputs); 

  
int main(){
	train_set_t train_set;
	set_train_set(train_set);
	print_set(train_set);
	Neuron neuron;
	neuron.init(2);
	// comment/uncomment functions 
	//manual_entry(neuron,train_set);
	//global_search(neuron,train_set);
   	gradient_search(neuron,train_set);
   	save_vector_to_file(convergence);
   	//draw_output(neuron); // use if you want
	 
 } 
