#include <routing_table.h>
#include <stdio.h>
#include <structs.h>
void modMatrix(int n_vertex,int matrix[n_vertex][n_vertex],int id){
	int aux[n_vertex],aux2,i,j;
	if(id!=1){
		for(j=0;j<n_vertex;j++){
			aux[j] = matrix[id-1][j];
			matrix[id-1][j] = matrix[0][j];
			matrix[0][j] = aux[j];
		}
		for(i=0;i<n_vertex;i++){
			aux2 = matrix[i][id-1];
			matrix[i][id-1] = matrix[i][0];
			matrix[i][0] = aux2;
		}
	}
}
int modMatrix2(int n_vertex,int matrix[n_vertex][n_vertex],int id2){
	int aux[n_vertex],aux2,i,j,i2;
		for(j=0;j<n_vertex;j++){
			aux[j] = matrix[id2-1][j];
			matrix[id2-1][j] = matrix[n_vertex-1][j];
			matrix[n_vertex-1][j] = aux[j];
		}
		for(i=0;i<n_vertex;i++){
			aux2 = matrix[i][id2-1];
			matrix[i][id2-1] = matrix[i][n_vertex-1];
			matrix[i][n_vertex-1] = aux2;
		}
}
int dk(int n_vertex,int id_origem,int id_destino,int matriz[n_vertex][n_vertex], int next){
    int i, j,anterior[n_vertex],dist[n_vertex],cont_fechados = 0,abertos[n_vertex];
    int aux = 1000000,contador=0;
    for(i = 0;i<n_vertex;i++){
    	if(i==0){
    		dist[i] = 0;
    	}
    	else{
    		dist[i] = 10000;
    	}
    	abertos[i] = 0;
    	anterior[i] = 0;
    }
    do{

        aux = 1000000;
        for(i=0;i<n_vertex;i++){
            if(abertos[i] == 0){
                if(dist[i]<aux){
                    aux = i;
                }
            }
        }
        abertos[aux] = 1;
        for(j=0;j<n_vertex;j++){
            if(matriz[aux][j]>0){
                if(abertos[j] == 0){
                    if(dist[aux] + matriz[aux][j] < dist[j]){
                        dist[j] = dist[aux] + matriz[aux][j];
                        anterior[j] = aux;
                    }
                }
                if((dist[j]+matriz[aux][j]) < dist[aux]){
                  dist[aux] = dist[j]+matriz[aux][j];
                  anterior[aux] = j;
                }
            }
        }
        contador++;
    }while(contador < n_vertex);
	int a=n_vertex-1,last;

    if(id_origem == n_vertex && id_destino ==1){
    	last = anterior[a];
    }else if(anterior[a] == 0){
    	last = id_destino - 1;
    }else{
    	do{
        	last = a;
        	a=anterior[a];
    	}while(a!=0);
    }

    printf("Caminho:%d\n",last + 1);
    printf("Destino: %d\n",id_destino);
    printf("Distancia: %d\n", dist[n_vertex-1]);
    printf("\n\n");
    routing_table[next].id_destination = id_destino;
    routing_table[next].id_next = last + 1;
    routing_table[next].cost = dist[n_vertex-1];
}

void create_routing_table(int id, int n_vertex){
	int i,j,i2;
	int aux;
    int next = 0;
  int source,dest,dist;
	int matrix[n_vertex][n_vertex];
	int auxM[n_vertex][n_vertex];
	for(i=0;i<n_vertex;i++){
		for(j=0;j<n_vertex;j++){
			matrix[i][j] = 0;
		}
	}
	FILE *file = fopen("config/enlaces.config","r");
	for(;fscanf(file,"%d %d %d",&source,&dest,&dist) != EOF;){
		matrix[source-1][dest-1] = dist;
		matrix[dest-1][source-1] = dist;
	}
	if(id != 1){
		modMatrix(n_vertex,matrix,id);
	}
	for(i=2;i<=n_vertex;i++){
		for(i2=0;i2<n_vertex;i2++){
			for(j=0;j<n_vertex;j++){
				auxM[i2][j] = matrix[i2][j];
			}
		}
		if(id == n_vertex && i == n_vertex){
			modMatrix2(n_vertex,auxM,1);
		}else{
			modMatrix2(n_vertex,auxM,i);
		}
		if(i != id){
			dk(n_vertex,id,i,auxM, next++);
		}
		else{
			dk(n_vertex,id,1,auxM, next++);
		}

	}
}
