/*
 * 
 * */

#include <cmath>
#include "image_pr4.h" 
#include <thread>
#include <chrono>


int rCount = 0; 
int gCount = 0;
int bCount = 0;
bool day = false;
int redPixelCount = 0;

int xStart, yStart, xEnd, yEnd; // the x&y values of top and bottom of the sun
int Gx[3][3] = {{-1, 0, 1},   {-2, 0, 2}, {-1, 0, 1}};
int Gy[3][3] = {{-1, -2, -1}, {0, 0, 0},  {1, 2, 1}};

using namespace std;



struct Orbit {
	// logged position and time
	std::vector<int> x;   // fill up until 30
	std::vector<int> y;
	std::vector<int> t;
	int xc,yc,r;  // center and radius
	int x_sunrise,y_sunrise;
	double omega = 0.1;
} orbit;




// finding the sun for challenge
void findEdges(int time){
	
	int height = image.height;
	int width = image.width;
	
	vector<int> edgeX; // stores all the x values of the edges
	vector<int> edgeY; // stores all the y values of the edges
	
	
	// going through every pixel that is not on the edges
	for(int row = 1; row < height - 1; row++){
		for(int col = 1; col < width - 1; col++){
			
			
			// determining whether it is a red pixel or not
			int rCount = int(get_pixel(image, row, col, 0));
			int gCount = int(get_pixel(image, row, col, 1));
			int bCount = int(get_pixel(image, row, col, 2));


			// if it is a red pixel, then determine the edges
			if ((rCount > 1.4 * gCount) && (rCount > 1.4 * bCount)){ 
				int GxSum = 0;
				int GySum = 0;
				for(int i=0; i<3; i++){
					for(int j=0; j<3; j++){
						GxSum = GxSum + get_pixel(image, (row-1) + i, (col-1) + j, 0)*Gx[i][j];
						GySum = GySum + get_pixel(image, (row-1) + i, (col-1) + j, 0)*Gy[i][j];
					}
				}
				
				// if it is an edge (greater than the threshold), then store the point
				double totalSum = sqrt(GxSum*GxSum + GySum*GySum);
				if(totalSum > 80){ 
					edgeX.push_back(col); // stores the x coordinate of an edge point
					edgeY.push_back(row); // stores the y coordinate of an edge point
				}
			}
		}
	}
	
	
	// makes a counting board of all the pixels, fills it up with 0 (just in case)
	int acc[height][width]; 
	for(int i=0; i<height; i++){
		for(int j=0; j<width; j++){ acc[i][j] = 0; }
	}
	
	// iterates through all the edges in the image
	// check xi xj and xk are too far apart
	// check y1 yj yk are too far apart
	
	for(unsigned int i = 0; i< edgeX.size(); i++){
		for(unsigned int j = i; j < edgeX.size(); j++){
			for(unsigned int k = j; k < edgeX.size(); k++){
				
				if(abs(edgeX[i] - edgeX[j]) > 30 && 
				abs(edgeX[i] - edgeX[k]) > 30 && 
				abs(edgeX[j] - edgeX[k]) > 30){
					continue;
				}
				
				if(abs(edgeY[i] - edgeY[j]) > 30 && 
				abs(edgeY[i] - edgeY[k]) > 30 && 
				abs(edgeY[j] - edgeY[k]) > 30){
					continue;
				}
				
				double a = 2*edgeX[i] - 2*edgeX[j];
				double b = 2*edgeY[i] - 2*edgeY[j];
				double c = pow(edgeX[j], 2) - pow(edgeX[i], 2) + pow(edgeY[j], 2) - pow(edgeY[i], 2);
				
				double d = 2*edgeX[i] - 2*edgeX[k];
				double e = 2*edgeY[i] - 2*edgeY[k];
				double f = pow(edgeX[k], 2) - pow(edgeX[i], 2) + pow(edgeY[k], 2) - pow(edgeY[i], 2);
				
				
				if(a*e - b*d != 0){
					double xc = (-1*c*e + f*b) / (a*e - b*d);  // where we think the center x is
					double yc = (-1*a*f + c*d) / (a*e - b*d);  // where we think the center y is
					if(int(yc) < 900 && int(yc) >= 0 && int(xc) < 900 && int(xc) >= 0){
						acc[int(yc)][int(xc)]++;	
					}
				}
			}
		}
	}
	

	// finding what pixel actually has the most votes
	for(int row = 0; row < height; row++){
		for(int col = 0; col < width; col++){
			if(acc[row][col] > 29000){
				
				// log the values into the x and y vector
				orbit.x.push_back(col);
				orbit.y.push_back(row);
				orbit.t.push_back(time);
				break;
			}
		}
	}
}








// function that returns the determinant of a matrix
double findDeterminant(double a, double b, double c, double d, double e, double f, double g, double h, double i){
	double determinant = a*(e*i - f*h) - b*(d*i - f*g) + c*(d*h - e*g);
	return determinant;
}



// determines circle parameters
void findOrbit(){
	int n = orbit.x.size();
	
	// finding all the a1 values
	double a11 = n;
	double a12 = 0;
	double a13 = 0;
	for(int i = 0; i < n; i++){ a13 += cos(orbit.omega * orbit.t[i]); }
	
	// finding all the a2 values
	double a21 = 0;
	double a22 = n;
	double a23 = 0;
	for(int i = 0; i < n; i++){ a23 += sin(orbit.omega * orbit.t[i]); }
	
	// finding all the a3 values
	double a31 = 0;
	for(int i = 0; i < n; i++){ a31 += cos(orbit.omega * orbit.t[i]); }
	double a32 = 0;
	for(int i = 0; i < n; i++){ a32 += sin(orbit.omega * orbit.t[i]); }
	double a33 = n;
	
	double b1 = 0;
	double b2 = 0;
	for(int i = 0; i < n; i++){ b1 += orbit.x[i]; }
	for(int i = 0; i < n; i++){ b2 += orbit.y[i]; }
	
	// part a and b of b3
	double b3a = 0;
	double b3b = 0;
	for(int i = 0; i < n; i++){ b3a += orbit.x[i] * cos(orbit.omega * orbit.t[i]); }
	for(int i = 0; i < n; i++){ b3b += orbit.y[i] * sin(orbit.omega * orbit.t[i]); }
	double b3 = b3a + b3b;
	
	double det = findDeterminant(a11, a12, a13, 
							     a21, a22, a23, 
							     a31, a32, a33);   // original 
							   
	double detXc = findDeterminant(b1, a12, a13,
								   b2, a22, a23,
								   b3, a32, a33);  // first column changed
								
	double detYc = findDeterminant(a11, b1, a13, 
								   a21, b2, a23, 
								   a31, b3, a33);  // second column changed								

	double detR = findDeterminant(a11, a12, b1, 
								  a21, a22, b2, 
								  a31, a32, b3);   // third column changed 
	
	cout << "det = " << det << endl;
	cout << "detXc = " << detXc << endl;
	cout << "detYc = " << detYc << endl;
	cout << "detR = " << detR << endl;
	
	
	orbit.xc = (detXc / det);  // xc
	orbit.yc = (detYc / det);  // yc
	orbit.r =  (detR / det);   // radius
}


// core code!!!
void findSun(int time){
    int height = image.height;
    int width = image.width;
	for (int row = 0; row < height; row++){
		for (int col = 0; col < width; col++){
			rCount = int(get_pixel(image, row, col, 0));
		    gCount = int(get_pixel(image, row, col, 1));
		    bCount = int(get_pixel(image, row, col, 2));
		  
			if ((rCount > 1.4 * gCount) && (rCount > 1.4 * bCount)){ 
				redPixelCount++; 
			    if(redPixelCount == 1){ 
					xStart = col;
				    yStart = row;
				}
				else {
					xEnd = col;
					yEnd = row;
				}
			}
		}
	} 
   if(redPixelCount > 100){ day = true; } // the sun is on the screen
   else { day = false; }
  
   if(day){
 	   int xCenter = xStart + (xEnd - xStart)/2;
	   int yCenter = yStart + (yEnd - yStart)/2;
	   orbit.x.push_back(xCenter);
	   orbit.y.push_back(yCenter);  
	   orbit.t.push_back(time);
   }
   redPixelCount = 0;
}




int main(){        
	std::cout<<"Start..."<<std::endl;
	init(2);
    for ( int time = 0 ; time < 950; time++){
      draw_all(time); 
      std::cout<<"Tick"<<std::endl;
      

      
      if(orbit.x.size() < 30){
		  //findSun(time);   // core and completion
		  findEdges(time); // challenge :(
      }
      else{
		  // there's enough data and can start following/predicting the sun
		  findOrbit();
		  double x = orbit.xc + orbit.r * cos(orbit.omega * time);
		  double y = orbit.yc + orbit.r * sin(orbit.omega * time);
		  double power = pow((image.height - orbit.yc), 2);
		  double xSunrise = orbit.xc - sqrt(abs(orbit.r - power)); 
		  orbit.x_sunrise = xSunrise;
		  orbit.y_sunrise = image.height;
		  
		  cout << "xc = " << orbit.xc << endl;
		  cout << "yc = " << orbit.yc << endl;
		  cout << "radius = " << orbit.r << endl;

		  
		  if(y > image.height){ day = false; }
		  else { day = true; }
		  if(day){
			  // the sun is on the screen and so we calculate x and y
			  int xPanel = 0;
			  int yPanel = 0;
			  get_aim(xPanel, yPanel);
			  double angle = atan2(y - yPanel, x - xPanel);
			  move_aim(angle);

		  }
		  else if(!day){ 
			  int xPanel = 0;
			  int yPanel = 0;
			  get_aim(xPanel, yPanel);
			  double angle = atan2(orbit.y_sunrise - yPanel, orbit.x_sunrise - xPanel);
			  move_aim(angle);
		  }
	  }
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
   }

    return 0;
}

