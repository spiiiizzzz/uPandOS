#include "./headers/msg.h"

static msg_t msgTable[MAXMESSAGES];
LIST_HEAD(msgFree_h);


/*Inizializza la lista msgFree per contenere tutti gli elementi dell'array statico
di lunghezza MAXMESSAGES*/
void initMsgs() {
    for(int i=0; i<MAXMESSAGES; i++){
        list_add(&(msgTable[i].m_list), &msgFree_h);
    }
}

/*inserisci l'elemento puntato da p nella lista msgFree*/
void freeMsg(msg_t *m) {
     list_add(&(m -> m_list), &msgFree_h); 
}

/*ritorna NULL se msgFree è vuota, altrimenti rimuove un elemento dalla lista msgFree,
inizializza i valori di tutti i campi e ritorna un puntatore all'elemento rimosso*/
msg_t *allocMsg() {
    if(list_empty(&msgFree_h)) return NULL;
    else{
        msg_t* to_allocate = container_of(list_next(&msgFree_h), msg_t, m_list);
        list_del(list_next(&msgFree_h));

        INIT_LIST_HEAD(&to_allocate -> m_list);

        to_allocate->m_sender = NULL;

        to_allocate->m_payload = 0;

        return to_allocate;
    }
}
   
/*inizializza una variabile per essere una coda di messaggi*/
void mkEmptyMessageQ(struct list_head *head) {
    INIT_LIST_HEAD(head);  
}

/*ritorna TRUE se la lista head è vuota, ritorna FALSE altrimenti*/
int emptyMessageQ(struct list_head *head) {
    return(list_empty(head));
}

/*inserisci il msg puntato da m alla coda della lista di messaggi puntata da head*/
void insertMessage(struct list_head *head, msg_t *m) {
    list_add_tail(&(m->m_list), head);
}

/*inserisci il msg puntato da m alla testa della lista di messaggi puntata da head*/
void pushMessage(struct list_head *head, msg_t *m) {
    list_add(&(m->m_list), head);
}

/*rimuove il primo elemento (partendo dalla testa) dalla lista di messaggi a cui si ha accesso
tramite head e il cui mittente è p_ptr e ritorna l'elemento rimosso, ritorna NULL se la lista è vuota
o non c'è un messaggio che fa match, e ritorna il primo messaggio se p_ptr è NULL*/
msg_t *popMessage(struct list_head *head, pcb_t *p_ptr) {
    if (list_empty(head)) {
        return NULL; 
    }

    msg_t *current;
    list_for_each_entry(current, head, m_list) {
        if (p_ptr == NULL || (current->m_sender) == p_ptr) {
            list_del(&(current->m_list));
            INIT_LIST_HEAD(&(current->m_list));
            return current;
        }
    }

    return NULL; 
}

/*ritorna un puntatore al primo messaggio della lista puntata da head,
ritorna NULL se la lista è vuota*/
msg_t *headMessage(struct list_head *head) {
    if (list_empty(head)) {
        return NULL; 
    }

    return container_of(list_next(head), msg_t, m_list);
}