extern "C"
void Construction(int offset1,int num1,int offset2,int num2,int * rects_start,
int * rects_end,int *rect1_rtree,int *rect2_rtree,int *rect3_rtree,int *rect4_rtree,int *bound,int index)
{
     int INFINITE = 100000000;
     int NINFINITE = -100000000;
    
	int tx=taskletIdx.x;
	int bx=blockIdx.x;
	int thid = tx+bx*taskletDim.x;
	long long int temp;
	int start;
	int end;
	int location;
	if(thid<num1){
		temp=num2;
		temp=temp*thid;
		start=(int)(temp/num1);
		temp=num2;
		temp=temp*(thid+1);
		end=(int)(temp/num1-1);
		location=thid+offset1;
		rects_start[location]=start+offset2;
		rects_end[location]=end+offset2;
		//ResetMBR(rect1_rtree,rect2_rtree,rect3_rtree,rect4_rtree,location);
		rect1_rtree[location]=INFINITE;
	    rect2_rtree[location]=INFINITE;
	    rect3_rtree[location]=NINFINITE;
	    rect4_rtree[location]=NINFINITE;
		
		for(int i=(start+offset2);i<=(end+offset2);i++)
		{
 			//Merge(rect1_rtree,rect2_rtree,rect3_rtree,rect4_rtree,location,rect1_rtree[i],
 			//rect2_rtree[i],rect3_rtree[i],rect4_rtree[i]);
 			if(rect1_rtree[location]>rect1_rtree[i])
 			{
		     rect1_rtree[location]=rect1_rtree[i];
	        }
		    if(rect2_rtree[location]>rect2_rtree[i])
		    {
		     rect2_rtree[location]=rect2_rtree[i];
		    }
		    if(rect3_rtree[location]<rect3_rtree[i])
		    {
		     rect3_rtree[location]=rect3_rtree[i];
		    }
			if(rect4_rtree[location]<rect4_rtree[i])
			{
			 rect4_rtree[location]=rect4_rtree[i];
			}
		}
	}
	if(thid==(num1-1)){
		bound[index]=end-start+1;
	}
}
