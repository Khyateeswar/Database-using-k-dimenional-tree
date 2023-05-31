#include <iostream>
#include <fstream>
#include "constants.h"
#include "file_manager.h"
#include "errors.h"
#include <cstring>
#include <bits/stdc++.h>

using namespace std;

void printl(vector<int> l){
    int n=l.size();
    for(int i =0;i<n;i++){
        cout<<l[i]<<" ";
    }
    cout<<" "<<endl;
    return;
}

vector<string> split(string s){
    string d="";
    int n= s.length();
    vector<string> l;
    for(int i=0;i<n;i++){
        if(s[i]==' '){
            l.push_back(d);
            d="";
        }else{
            d=d+s[i];
            if(i==(n-1)){
                l.push_back(d);
            }
        }
    }
    return l;
}

int GetInt(PageHandler ph,int i){//checked
    int num;
    char* data = ph.GetData();
    memcpy (&num, &data[i], sizeof(int));
    return num;
}

void SetInt(FileHandler &fh,PageHandler ph,int a,int i){//checked
    int num = a;
    PageHandler pp = fh.PageAt(ph.GetPageNum());
    char* data =pp.GetData();
    memcpy (&data[i], &num, sizeof(int));
    return;
}

vector<int> GetList(PageHandler ph,int i, int size){
    vector<int> v;
    for(int j=i;j<i+4*size;j=j+4){
        int h = GetInt(ph,j);
        v.push_back(h);
    }
    return v;
}

void SetList(FileHandler &fh,PageHandler ph,vector<int> l,int i){
    int n = l.size();
    for(int j=0;j<4*n;j=j+4){
        SetInt(fh,ph,l[j/4],i+j);
    }
    return;
}

bool Check_reg(PageHandler ph){//checked
    if (GetInt(ph,4)== 0){
        return true;
    }
    return false;
}

int Get_dim(PageHandler ph){//chrcked
    return GetInt(ph,8);
}

int Get_ndim(PageHandler ph,int dim){
    return (Get_dim(ph)+1)%(dim);
}

int Get_par(PageHandler ph){//checked
    return GetInt(ph,12);
}

int Get_end(PageHandler ph,int d){
    if(Check_reg(ph)){
        int i =16;
        int df = 4+8*d;
        int check = 0;
        while(GetInt(ph,i)!=0){
            i=i+df;
        }
        return i;
    }else{
        int i =16;
        int df = 4+4*d;
        while(GetInt(ph,i+4*d)!=0){
            i=i+df;
        }
        return i;
    }
}

void print(PageHandler ph,int d){
    cout<<"              "<<endl;
    cout<<GetInt(ph,0)<<endl;//page num
    cout<<GetInt(ph,4)<<endl;//typ
    cout<<GetInt(ph,8)<<endl;//split dim
    cout<<GetInt(ph,12)<<endl;//parent
    if(Check_reg(ph)){

        int i = 16;
        while(i<Get_end(ph,d)){
            cout<<GetInt(ph,i)<<endl;
            for(int j=0;j<8*d;j=j+4){
                cout<<GetInt(ph,i+j+4)<<" ";
            }
            cout<<" "<<endl;
            i=i+8*d+4;
        }
        
        cout<<" "<<endl;
    }else{
        int i = 16;
        while(i<Get_end(ph,d)){
            for(int j=0;j<4*d;j=j+4){
                cout<<GetInt(ph,i+j)<<" ";
            }
            cout<<" "<<endl;
            cout<<GetInt(ph,i+4*d)<<endl;
            i=i+4+4*d;
        }
        cout<<" "<<endl;
        
    }
    

}

bool overflow(PageHandler ph,int d){//checked
    if(Check_reg(ph)){
        if(Get_end(ph,d)+2*(8*d+4)  < PAGE_CONTENT_SIZE){
            return false;
        }
        return true;

    }else{
        if(Get_end(ph,d)+(4+4*d) < PAGE_CONTENT_SIZE){
        return false;
    }
    return true;
    }
}

PageHandler CreateRootNode(FileHandler &fh,int dim){//checked
    PageHandler ph = fh.NewPage();
    SetInt(fh,ph,ph.GetPageNum(),0);
    SetInt(fh,ph,0,4);
    SetInt(fh,ph,0,8);
    SetInt(fh,ph,-1,12);
    SetInt(fh,ph,1,16);
    for(int i =0;i<8*dim;i=i+8){
        SetInt(fh,ph,-1000000,i+20);
        SetInt(fh,ph,1000000,i+24);
    }
    return ph;
}

PageHandler CreateRegNode(FileHandler &fh ,int dim,int node_parent,int split_dim){//checked
    // first 4 bytes - pagenumber
    //  second 4 bytes - type of node 0 for region
    //  third  4 bytes - split_dim
    //  fourth 4 bytes - node_parent page number
    //  child id =  page number of the child
    //   Range of child region is given for d dim = a1_min,a1_max,a2_min,....
    PageHandler ph = fh.NewPage();
    SetInt(fh,ph,ph.GetPageNum(),0);
    SetInt(fh,ph,0,4);
    SetInt(fh,ph,split_dim,8);
    SetInt(fh,ph,node_parent,12);
    return ph;
}

PageHandler CreatePoiNode(FileHandler &fh,int dim,int node_parent,int split_dim){//checked
    // first 4 bytes - page number
    //  second 4 bytes - type of node 1 for point node
    //  third 4 bytes - split-dim
    //  fourth 4 bytes - node parent page number
    //   d-dim point + 1 for locaiton stored sequentially
    PageHandler ph = fh.NewPage();
    SetInt(fh,ph,ph.GetPageNum(),0);
    SetInt(fh,ph,1,4);
    SetInt(fh,ph,split_dim,8);
    SetInt(fh,ph,node_parent,12);
    return ph;
}

vector<int> ChooseSplit(PageHandler ph,int d){//checked
    int dim = Get_dim(ph);
    if(Check_reg(ph)){
        vector<int> v;
        int i = 16;
        while(i<Get_end(ph,d)){
            i=i+4;
            v.push_back(GetInt(ph,i+8*dim));
            i=i+8*d;
        }
        sort(v.begin(),v.end());
        int n = v.size();
        int k;
        k=v[n/2];
        vector<int> b;
        b.push_back(k);
        b.push_back(dim);
        return b;
    }else{
        vector<int> v;
        int i=16;
        while(i<Get_end(ph,d)){
            v.push_back(GetInt(ph,i+4*dim));
            i=i+4+4*d;
        }
        sort(v.begin(),v.end());
        int n = v.size();
        int k;
        k=v[n/2];
        vector<int> b;
        b.push_back(k);
        b.push_back(dim);
        return b;
    }
}

vector<int> NodeSplit(FileHandler &fh,PageHandler ph,vector<int> split_ele,int d){
    //cout<<"Nodesplit started"<<endl;
    int k = split_ele[0];
    int split_d = split_ele[1];
    if(Check_reg(ph)){//region node splitting
        PageHandler lef = CreateRegNode(fh,d,Get_par(ph),Get_ndim(ph,d));
        PageHandler rig = CreateRegNode(fh,d,Get_par(ph),Get_ndim(ph,d));
        //cout<<"Created lef and rig regnodes for splitting in nodesplit  "<< lef.GetPageNum()<<"  " << rig.GetPageNum()<<endl;
        //print(lef,d);
        //print(rig,d);
        int l_end = Get_end(lef,d);
        int r_end =  Get_end(rig,d);
        int i=16;
        int sp_dim = 20+8*split_d;
        while(i<Get_end(ph,d)){
            //cout<<"Nodesplit for reg node"<<endl;
            int childid = GetInt(ph,i);
            if(GetInt(ph,sp_dim)<k && GetInt(ph,sp_dim+4)<k){
                vector<int> v=GetList(ph,i,2*d+1);
                SetList(fh,lef,v,l_end);
            }else if(GetInt(ph,sp_dim)>=k && GetInt(ph,sp_dim+4)>k){
                vector<int> v=GetList(ph,i,2*d+1);
                SetList(fh,rig,v,r_end);
            }else{
                PageHandler ph_chld = fh.PageAt(childid);
                //print(ph_chld,d);
                //cout<<childid<<endl;
                vector<int> split_child = NodeSplit(fh,ph_chld,split_ele,d);
                //cout<<" return of recursive nodesplit"<<endl;
                int lef_chld =  split_child[0];
                int rig_chld =  split_child[1];
                PageHandler lef_split = fh.PageAt(lef_chld);
                PageHandler rig_split = fh.PageAt(rig_chld);
                //print(lef_split,d);
                //print(rig_split,d);
                //cout<<"end points of lef and rig "<<l_end<<" "<<r_end<<endl;
                SetInt(fh,lef_split,lef.GetPageNum(),12);//parent set 
                SetInt(fh,rig_split,rig.GetPageNum(),12);//parent set
                //cout<<" lef split parent "<< lef.GetPageNum()<<" rig split parent "<<rig.GetPageNum()<<endl;
                vector<int> parent = GetList(ph,i,2*d+1);
                SetList(fh,lef,parent,l_end);
                SetList(fh,rig,parent,r_end);
                SetInt(fh,lef,lef_chld,l_end);//child id set
                SetInt(fh,rig,rig_chld,r_end);//child id set
                //cout<<" lef split region "<< lef_chld<<" rig split region "<<rig_chld<<endl;
                //Set the certain dimension 
                SetInt(fh,lef,k,l_end+8*split_d+4+4);
                SetInt(fh,rig,k,r_end+8*split_d+4);
                fh.MarkDirty(lef_chld);
                fh.UnpinPage(lef_chld);
                fh.MarkDirty(rig_chld);
                fh.UnpinPage(rig_chld);
                if(fh.DisposePage(childid)){
                    //removing child page
                }
            }
            sp_dim = sp_dim+8*d+4;
            l_end = Get_end(lef,d);
            r_end =  Get_end(rig,d);
            i = i+8*d+4;
        }
        vector<int> nodesplit;
        nodesplit.push_back(lef.GetPageNum());
        nodesplit.push_back(rig.GetPageNum());
        //print(lef,d);
        //print(rig,d);
        return nodesplit;

    }else{//Point node splitting
        //cout<<"Nodesplit went to base case"<<endl;
        PageHandler lef = CreatePoiNode(fh,d,Get_par(ph),Get_ndim(ph,d));
        PageHandler rig = CreatePoiNode(fh,d,Get_par(ph),Get_ndim(ph,d));
        //cout<<"Created lef and rig poinodes for nodesplit"<< lef.GetPageNum()<< " "<<rig.GetPageNum() <<endl;
        int l_end = Get_end(lef,d);
        int r_end =  Get_end(rig,d);
        int i=16;
        int g = 16+4*split_d;
        while(i<Get_end(ph,d)){
            int childid = GetInt(ph,i);
            if(GetInt(ph,g)<k){
                vector<int> v=GetList(ph,i,d+1);
                SetList(fh,lef,v,l_end);
            }else{
                vector<int> v=GetList(ph,i,d+1);
                SetList(fh,rig,v,r_end);
            }
            g=g+4*d+4;
            l_end = Get_end(lef,d);
            r_end = Get_end(rig,d);
            i=i+4*d+4;
        }
        vector<int> nodesplit;
        nodesplit.push_back(lef.GetPageNum());
        nodesplit.push_back(rig.GetPageNum());
        //cout<<"return base case of nodesplit "<<endl;
        return nodesplit;
        
    }
}

void Reorg(FileHandler &fh,PageHandler ph,int d){
    //cout<<"Reorg initialised"<<endl;
    int pg = ph.GetPageNum();
    int pg_par = Get_par(ph);
    PageHandler par ;
    if(pg_par!=0){
        par = fh.PageAt(pg_par);
    }
    //cout<<pg_par<<endl;
    vector<int> split_ele = ChooseSplit(ph,d);
    //cout<<"Choosesplit success"<<endl;
    int sep = split_ele[0];
    int sp_dim = split_ele[1];
    vector<int> nodes = NodeSplit(fh,ph,split_ele,d);
    //cout<<"nodesplit success"<<endl;
    int lef_pg = nodes[0];
    int rig_pg = nodes[1];
    PageHandler lef = fh.PageAt(lef_pg);
    PageHandler rig = fh.PageAt(rig_pg);

    if(pg_par==0){
        //cout<<"root reorg "<<endl;
        PageHandler root = CreateRegNode(fh,d,0,Get_ndim(ph,d));
        PageHandler root_sen = fh.PageAt(0);
        SetInt(fh,root_sen,root.GetPageNum(),16);
        int root_end = Get_end(root,d);
        vector<int> temp = GetList(root_sen,root_end,16);
        SetList(fh,root,temp,root_end);
        SetInt(fh,root,lef_pg,root_end);
        SetInt(fh,root,sep,root_end+4+8*sp_dim+4);
        root_end = Get_end(root,d);
        SetList(fh,root,temp,root_end);
        SetInt(fh,root,rig_pg,root_end);
        SetInt(fh,root,sep,root_end+4+8*sp_dim);
        SetInt(fh,lef,root.GetPageNum(),12);//stet parent
        SetInt(fh,rig,root.GetPageNum(),12);
        fh.MarkDirty(root.GetPageNum());
        fh.UnpinPage(root.GetPageNum());
        if(fh.DisposePage(ph.GetPageNum())){
            //reomoving waste page
        }
        return;

    }else{
        //cout<<"not root reorg"<<endl;
        int i =16;
        vector<int> l;
        while(i<Get_end(par,d)){
            //cout<<"not root reorg while inside" <<endl;
            if(GetInt(par,i)==pg){
                l=GetList(par,i,2*d+1);
                //printl(l);
                SetList(fh,par,l,i);
                break;
            }else{
                i=i+8*d+4;
            }
        }
        //cout<<"not root reorg while done" <<endl;
        int par_end = Get_end(par,d);
        SetList(fh,par,l,par_end);
        SetInt(fh,par,lef_pg,i);//set childid
        SetInt(fh,par,rig_pg,par_end);//set childid
        SetInt(fh,par,sep,i+4+8*sp_dim+4);//region set
        SetInt(fh,par,sep,par_end+4+8*sp_dim);//region set
        if(fh.DisposePage(ph.GetPageNum())){
            //removing reorg page
        }
        if(overflow(par,d)){
            Reorg(fh,par,d);
        }
        return;
    }
    //cout<<"Reorg success"<<endl;
}

void Insert(FileHandler &fh, vector <int> l,int d ){
    PageHandler ph = fh.PageAt(0);
    while(Check_reg(ph)){
        int i =16;
        bool nfound = true;
        while(i<Get_end(ph,d) && nfound ){
            print(ph,d);
            int chld = GetInt(ph,i);
            i=i+4;
            int check = 1;
            for(int j=0;j<8*d;j=j+8){
                if(l[j/8]>=GetInt(ph,i) && l[j/8]<GetInt(ph,i+4)){
                    check = check*1;
                }else{
                    check = check*0;
                }
                i=i+8;
            }
            if(check ==1){
                ph = fh.PageAt(chld);
                nfound = false;
            }
        }
    }
    //print(ph,d);
    if(!overflow(ph,d)){
        //cout<<"not overflow"<<endl;
        int p = Get_end(ph,d);
        for(int j =0;j<4*d;j=j+4){
        SetInt(fh,ph,l[j/4],p);
        p=p+4;
        }
        SetInt(fh,ph,-1,p);
        fh.MarkDirty(ph.GetPageNum());
        fh.UnpinPage(ph.GetPageNum());
    }else{
        //cout<<"overflow"<<endl;
        //cout<<ph.GetPageNum()<<endl;
        Reorg(fh,ph,d);
        //cout<<"insert reorg success"<<endl;
        Insert(fh,l,d);
    }
    /*
    cout<<"insert done"<<endl;
    cout<<" "<<endl;
    */
    return;
}

void Insert_out(FileHandler &fh, vector <int> l,int d ,ofstream & output){
    PageHandler ph = fh.PageAt(0);
    while(Check_reg(ph)){
        int i =16;
        bool nfound = true;
        while(i<Get_end(ph,d) && nfound ){
            int chld = GetInt(ph,i);
            i=i+4;
            int check = 1;
            for(int j=0;j<8*d;j=j+8){
                if(l[j/8]>=GetInt(ph,i) && l[j/8]<GetInt(ph,i+4)){
                    check = check*1;
                }else{
                    check = check*0;
                }
                i=i+8;
            }
            if(check ==1){
                fh.UnpinPage(ph.GetPageNum());
                ph = fh.PageAt(chld);
                nfound = false;
            }
        }
    }
    output<<"         \n";
    output<<"         \n";
    output<<"INSERTION DONE\n";
    int i = 16;
    while(i<Get_end(ph,d)){
        for(int j =0;j<4*d;j=j+4){
            output<<to_string(GetInt(ph,i+j))<<" ";
        }
        output<<"\n";
        i=i+4+4*d;
    }
    fh.UnpinPage(ph.GetPageNum());
    return;
}

void Pquery(FileHandler &fh, vector <int> l,int d,ofstream & output ){
    PageHandler ph = fh.PageAt(0);
    int count = -1;
    while(Check_reg(ph) ){
        int i =16;
        bool nfound = true;
        while(i<Get_end(ph,d) && nfound ){
            int chld = GetInt(ph,i);
            i=i+4;
            int check = 1;
            for(int j=0;j<8*d;j=j+8){
                if(l[j/8]>=GetInt(ph,i) && l[j/8]<GetInt(ph,i+4)){
                    check = check*1;
                }else{
                    check = check*0;
                }
                i=i+8;
            }
            if(check ==1){
                count++;
                fh.UnpinPage(ph.GetPageNum());
                ph = fh.PageAt(chld);
                nfound = false;
            }
        }
        if(nfound==true){
            output<<"         \n";
            output<<"         \n";
            output<<"NUM OF REGIONS TOUCHED:"<<" "<<to_string(0)<<"\n";
            output<<"FALSE\n";
        }
    }
    int i=16 ;
    bool nfound= true;
    while(i<Get_end(ph,d) && nfound){
        int check = 1;
        for(int j =0;j<d;j++){
            if(l[j]==GetInt(ph,i+4*j)){
                check=check*1;
            }else{
                check=check*0;
            }
        }
        if(check==1){
            nfound =false;
        }else{
            i=i+4*d+4;
        } 
    }
    fh.UnpinPage(ph.GetPageNum());
    if(nfound==true){
        output<<"         \n";
        output<<"         \n";
        output<<"NUM REGION NODES TOUCHED:"<<" "<<to_string(0)<<"\n";
        output<<"FALSE\n";

    }else{
        output<<"         \n";
        output<<"         \n";
        output<<"NUM REGION NODES TOUCHED:"<<" "<<to_string(count)<<"\n";
        output<<"TRUE\n";

    }

}

void Explore(FileHandler &fh,PageHandler ph,vector<int> l,int d,int count,ofstream & output){

    vector<int> min;
    vector<int> max;
    int pggg = ph.GetPageNum();
    //cout<<"Explore started on "<<pggg<<endl;
    for(int j=0;j<2*d;j++){
        if(j%2==0){
            min.push_back(l[j]);
        }else{
            max.push_back(l[j]);
        }
    }
    if(Check_reg(ph)){
        int i=16;
        while(i<Get_end(ph,d)){
            int pg_chld = GetInt(ph,i);
            PageHandler chld = fh.PageAt(pg_chld);
            i=i+4;
            vector<int> parent = GetList(ph,i,2*d);
            vector<int> r_min;
            vector<int> r_max;
            for(int j=0;j<2*d;j++){
                if(j%2==0){
                    r_min.push_back(parent[j]);
                }else{
                    r_max.push_back(parent[j]);
                }
            }
            int min_present = 1;
            for(int j=0;j<d;j++){
                if(r_min[j]<=min[j] && r_max[j]>min[j]){
                    min_present = min_present*1;
                }else{
                    min_present = min_present*0;
                }
            }
            int max_present = 1;
            for(int j=0;j<d;j++){
                if(r_min[j]<=max[j] && r_max[j]>max[j]){
                    max_present = max_present*1;
                }else{
                    max_present = max_present*0;
                }
            }
            
            if(min_present==1 && max_present==1){
                Explore(fh,chld,l,d,count+1,output);
            }else if(min_present==1){
                vector<int> final;
                for(int j=0;j<2*d;j++){
                    if(j%2==0){
                        final.push_back(min[j/2]);
                    }else{
                        final.push_back(r_max[j/2]);
                    }
                }
                Explore(fh,chld,final,d,count+1,output);
            }else if(max_present==1){
                vector<int> final;
                for(int j=0;j<2*d;j++){
                    if(j%2==0){
                        final.push_back(r_min[j/2]);
                    }else{
                        final.push_back(max[j/2]);
                    }
                }
                Explore(fh,chld,final,d,count+1,output);
            }
            i=i+8*d;
        }
    }else{
        int i=16;
        while(i<Get_end(ph,d)){
            int check =1;
            for(int j=0;j<d;j++){
                if(GetInt(ph,i+4*j)>=min[j]&& GetInt(ph,i+4*j)<max[j]){
                    check=check*1;
                }else{
                    check = check*0;
                }
            }
            if(check==1){
                output<<"POINT: ";
                for(int j=0;j<d;j++){
                    output<<to_string(GetInt(ph,i+4*j))<<" ";
                }
                output<<"NUM REGION NODES TOUCHED: ";
                output<<to_string(count);
                output<<"\n";
            }
            i=i+4*d+4;
        }
    }

}

void Rquery(FileHandler &fh, vector <int> l,int d,ofstream & output){
    output<<"\n";
    output<<"\n";
    cout<<"Rquery initialised"<<endl;
    PageHandler ph = fh.PageAt(0);
    int pg_child = GetInt(ph,16);
    ph = fh.PageAt(pg_child);
    Explore(fh,ph,l,d,0,output);
}


int main(int argc,char* argv[]){
   
    string inputf = argv[1];
    //cout<<inputf<<endl;
    string outputf = argv[3];
    //cout<<outputf<<endl;
    string d = argv[2];
    //cout<<d<<endl;
    FileManager kdb;
    FileHandler f = kdb.CreateFile("KDBTree.txt");
    //cout << "File KDBTree.txt created" << endl;
    int size = sizeof(int);
    int pg_size = PAGE_SIZE;
    int dim = stoi(d);
    string line;
    int query_no =0;
    ifstream query(inputf);
    ofstream output(outputf);
    if (query.is_open())
  {
    if (output.is_open()){
    while ( getline (query,line) )
    {
        if(!line.empty()){
        vector<string> l = split(line);
        string typ = l[0];
        if(query_no==0){
            cout<<dim<<endl;
            PageHandler root_sen = CreateRootNode(f,dim);
            int root_sen_pg = root_sen.GetPageNum();
            PageHandler root = CreateRootNode(f,dim);
            int root_page = root.GetPageNum();
            PageHandler root_point = CreatePoiNode(f,dim,root_page,0);
            int root_pointid = root_point.GetPageNum();
            SetInt(f,root,root_pointid,16);
            SetInt(f,root_sen,root_page,16);
            SetInt(f,root,root_sen_pg,12);
        }
        vector<int> k;
        for(int i=1;i<l.size();i++){
            k.push_back(stoi(l[i]));
        }
        //printl(k);
        if(typ=="INSERT"){
            Insert(f,k,dim);
            Insert_out(f,k,dim,output);
        }else if(typ=="PQUERY"){
            Pquery(f,k,dim,output);
        }else{
            Rquery(f,k,dim,output);
        }
        query_no++;
        //cout<<query_no<<endl;
        }
    }
    output.close();
  }
    query.close();
  
  }


    kdb.DestroyFile("KDBTree.txt");
    return 0;
}
