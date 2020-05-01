typedef struct List{
	char* path;
	List* next;

	List() {
		path = NULL;
		next = NULL;
	}

	List(char* path){
		this->path = path;
		next=NULL;
	}

	List(char* path, List* next){
		this->path = path;
		this->next = next;
	}

	~List(){}

    List* insert_first(List* s, /*param*/ d) {
        return new List(d,s);
    }

    void stampa(List *s){
	    List *q=s;

	    while(q!=NULL){
	    	q->stampa(); //Va implementato il metodo per il singolo valore
	    	q=q->next;    
	    }

	    printf("\n");
    }

}List;


int lung (List *s){

	int count=0;

	if(s==NULL) return count;

	List *q=s;


	while(q!=NULL){

		count++;
		q=q->next;

	}


	return count;
}


List *insert_last(List*s, Tdato d){

	if(s==NULL) return new List (d);

	List *q=s;


	while(q->next!=NULL){

		q=q->next;
	
	}


	q->next=new List(d);

	return s;

}


List * remove_first (List* s){

	if (s==NULL){

		return s;

	}

	List* n=s->next;

	delete s;
	return n;	
}


List * remove_last (List *s){

	if (s==NULL) return s;


	List *q=s;


	if (q->next==NULL){
		
		delete s;
		return NULL;
	}


	while(q->next->next!=NULL){

		q=q->next;
	
	}


	delete q->next;

	q->next=NULL;

	return s;
}


List * search_remove (List *p, int val){

	bool trovato=false;


	if(p==NULL){

		return p;
	
	}


	List* q=p;


	if (q->dato.index==val){

		return remove_first(p);

	}


	while (q->next!= NULL){
 
		if ( q->next->dato.index == val ){

			q->next= remove_first(q->next);

			return p;
		}

		q=q->next;

	}

	return p;

}


List * insert_order ( List *s, Tdato d ){

	if ( s == NULL || s->dato.gt(d.value) ) return insert_first(s, d);


	List *q=s;

	while (( q->next!=NULL ) && q->next->dato.lt(d.value) ){

		q = q -> next;

	}

	q->next=new List(d, q->next);

	return s;

}