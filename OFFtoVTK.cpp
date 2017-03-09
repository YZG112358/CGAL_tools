#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fstream>

class ChangeFormat{
public:
	//~ChangeFormat();
	ChangeFormat(){}
    ChangeFormat(ChangeFormat& cf){
	    int len = strlen(cf.in);
        this->in = (char*)malloc(len + 1);
        strcpy(this->in, cf.in);
        len = strlen(cf.out);
        this->out = (char*)malloc(len + 1);
        strcpy(this->out, cf.out);
	}

    ChangeFormat& operator=(ChangeFormat& cf){
	    int len = strlen(cf.in);
        this->in = (char*)malloc(len + 1);
        strcpy(this->in, cf.in);
        len = strlen(cf.out);
        this->out = (char*)malloc(len + 1);
        strcpy(this->out, cf.out);

        return *this;
    }
    void setOut(char* out){
        int len = strlen(out);
        this->out = (char*)malloc(len + 1);
        strcpy(this->out, out);
    }   
    void setIn(char* in){
        int len = strlen(in);
        this->in = (char*)malloc(len + 1);
        strcpy(this->in, in);
    }
    void OFFtoVTK();
private:
	char* in;
	char* out;
    size_t vertex_num;
    size_t face_num;
    size_t edge_num;
    void OFFset(FILE*);
    void VTKheader(FILE*);
    void VTKpolyhedron(FILE*);
};

void ChangeFormat::OFFset(FILE* off){
    char buffer[50];
    fscanf(off, "%s", buffer);
    fscanf(off, "%s", buffer);
    this->vertex_num = atoi(buffer);
    fscanf(off, "%s", buffer);
    this->face_num = atoi(buffer);
    fscanf(off, "%s", buffer);
    this->edge_num = atoi(buffer);
}

void ChangeFormat::VTKheader(FILE* vtk){
    fprintf(vtk, "# vtk DataFile Version 1.0\n");
    fprintf(vtk, "Just for test\n");
    fprintf(vtk, "ASCII\nDATASET POLYDATA\n");
    fprintf(vtk, "POINTS %zu float\n", this->vertex_num);
}

void ChangeFormat::VTKpolyhedron(FILE* vtk){
    fprintf(vtk,"POLYGONS %zu %zu\n", this->face_num, 4 * this->face_num);
}

void ChangeFormat::OFFtoVTK(){
    char buffer[50];
    FILE* in_file = fopen(this->in, "r");
    FILE* out_file = fopen(this->out, "w+");
    OFFset(in_file);
    VTKheader(out_file);    
    size_t v_times = 3 * this->vertex_num;
    for (size_t i = 0; i < v_times; i++){
        fscanf(in_file, "%s", buffer);
        fprintf(out_file, "%s ", buffer);
        if ((i%3) == 2){
            fprintf(out_file, "\n");
        }
    }
    VTKpolyhedron(out_file);   
    size_t f_times = 4 * this->face_num;
    for (size_t i = 0; i < f_times; i++){
        fscanf(in_file, "%s", buffer);
        fprintf(out_file, "%s ", buffer);
        if ((i%4) == 3){
            fprintf(out_file, "\n");
        }
    }
}

int main(int argc, char* argv[]){
    if (argc < 2){
        fprintf(stderr,"please input infile & outfile\n");
        exit(1);
    }
    ChangeFormat cf;
    cf.setIn(argv[1]);
    cf.setOut(argv[2]);
    cf.OFFtoVTK();
}

