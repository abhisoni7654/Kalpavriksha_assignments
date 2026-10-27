
#include<stdio.h>
#include<stdlib.h>

int getSmoothedValue(int **matrix,int *prevRow,int row,int col,int size,int leftVal){
    int sum=*(*(matrix+row)+col);
    int validCount=1;
    // to check if top row exists
    if(row>0){
        sum+=*(prevRow+col);
        validCount++;
        // top left diagonal
        if(col>0){
            sum+=*(prevRow+col-1);
            validCount++;
        }
        //top right diagonal
        if(col<size-1){
            sum+=*(prevRow+col+1);
            validCount++;
        }
    }
    // to check if bottom row exists
    if(row<size-1){
        sum+=*(*(matrix+row+1)+col);
        validCount++;
        // bottom left diagonal
        if(col>0){
            sum+=*(*(matrix+row+1)+col-1);
            validCount++;
        }
        // bottom right diagonal
        if(col<size-1){
            sum+=*(*(matrix+row+1)+col+1);
            validCount++;
        }
    }
    // check left value in same row
    if(col>0){
        sum+=leftVal;
        validCount++;
    }
    // check right value in same row
    if(col<size-1){
        sum+=*(*(matrix+row)+col+1);
        validCount++;
    }
    return sum/validCount;
}
void applyFilter(int **matrix,int size){
    // initialise prevRow dynamically to store original values of the previous row
    int *prevRow=calloc(size,sizeof(int));
    // original values of current row are saved into currRow as the row is traversed
    int *currRow=calloc(size,sizeof(int));
    // current value of cell is saved into leftVal as it would be needed by the element to it's right
    // initialised as 0 bcoz the leftmost element does not access it 
    int leftVal=0;
    for(int row=0;row<size;row++){
        for(int col=0;col<size;col++){
            *(currRow+col)=*(*(matrix+row)+col); // original value of cell saved to currRow
            int tempVal=*(*(matrix+row)+col); // value of current cell saved to tempVal
            *(*(matrix+row)+col)=getSmoothedValue(matrix,prevRow,row,col,size,leftVal);
            leftVal=tempVal; //current cell now becomes leftVal for next cell
        }
        //currRow now become prevRow so values are copied
        for(int col=0;col<size;col++){
            *(prevRow+col)=*(currRow+col);
        }
    }
    //allocated memory is freed
    free(currRow);
    free(prevRow);
}
void rotateClockwise(int **matrix,int size){
    // to rotate a matrix 90* clockwise we have to:
    // 1. take transpose of matrix
    // 2. reverse each row 

    // taking transpose of matrix
    for(int row=0;row<size;row++){
        for(int col=row;col<size;col++){
            int tempVal=*(*(matrix+row)+col);
            *(*(matrix+row)+col)=*(*(matrix+col)+row);
            *(*(matrix+col)+row)=tempVal;
        }
    }
    // reverse each row
    for(int row=0;row<size;row++){
        int startIdx=0;
        int endIdx=size-1;
        while(startIdx<endIdx){
            int tempVal=*(*(matrix+row)+startIdx);
            *(*(matrix+row)+startIdx)=*(*(matrix+row)+endIdx);
            *(*(matrix+row)+endIdx)=tempVal;
            startIdx++;
            endIdx--;
        }
    }
}
int main(){
    int **matrix;
    int size;
    printf("Enter size of matrix:");
    scanf("%d",&size);
    if(size<2||size>10){
        printf("Invalid size of matrix");
        return 1;
    }
    //dynamically allocate memory for 2d array
    matrix=(int **)calloc(size,sizeof(int *)); // this allocates enough space to create n row pointers
    // this loop allocates each row with space of array of size n
    for(int i=0;i<size;i++){
        *(matrix+i)=(int *)calloc(size,sizeof(int));
    }
    // generate random values of NxN matrix
    for(int i=0;i<size;i++){
        for(int j=0;j<size;j++){
            *(*(matrix+i)+j)=rand()%256;
        }
    }
    printf("Original randomly generated matrix:\n");
    for(int i=0;i<size;i++){
        for(int j=0;j<size;j++){
            printf("%d ",*(*(matrix+i)+j));
        }
        printf("\n");
    }
    rotateClockwise(matrix,size);
    printf("Matrix after 90* clockwise rotation:\n");
    for(int i=0;i<size;i++){
        for(int j=0;j<size;j++){
            printf("%d ",*(*(matrix+i)+j));
        }
        printf("\n");
    }
    applyFilter(matrix,size);
    printf("Matrix after applying 3x3 smoothening filter:\n");
    for(int i=0;i<size;i++){
        for(int j=0;j<size;j++){
            printf("%d ",*(*(matrix+i)+j));
        }
        printf("\n");
    }
    // free allocated memory of each row
    for(int i=0;i<size;i++){
        free(*(matrix+i));
    }
    // free all row pointers
    free(matrix);
    return 0;
}
