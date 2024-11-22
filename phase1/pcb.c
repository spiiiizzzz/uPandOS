#include "./headers/pcb.h"

static pcb_t pcbTable[MAXPROC];
LIST_HEAD(pcbFree_h);
static int next_pid = 1;

/*Inizializza la lista pcbFree per contenere tutti gli elementi dell'array statico
di lunghezza MAXPROC*/
void initPcbs() {
    for (int i = 0; i < MAXPROC; i++) {
        list_add(&(pcbTable[i].p_list), &pcbFree_h);
    }
}

/*inserisci l'elemento puntato da p nella lista pcbFree*/
void freePcb(pcb_t *p) {
    list_add(&(p->p_list), &pcbFree_h);
}

/*ritorna NULL se pcbFree è vuota, altrimenti rimuove un elemento dalla lista pcbFree,
inizializza i valori di tutti i campi e ritorna un puntatore all'elemento rimosso*/
pcb_t *allocPcb() {
    if (list_empty(&pcbFree_h)) {
        return NULL;
    } else {
        pcb_t* to_allocate = container_of(list_next(&pcbFree_h), pcb_t, p_list);
        list_del(list_next(&pcbFree_h));
        
        
        INIT_LIST_HEAD(&to_allocate->p_list);

        to_allocate -> p_parent = NULL;
        INIT_LIST_HEAD(&(to_allocate -> p_child));
        INIT_LIST_HEAD(&(to_allocate -> p_sib));

        (to_allocate -> p_s).entry_hi = 0;
        (to_allocate -> p_s).cause = 0;
        (to_allocate -> p_s).status = 0;
        (to_allocate -> p_s).pc_epc = 0;
        for (int i = 0; i < STATE_GPR_LEN; i++){
            (to_allocate -> p_s).gpr[i] = 0;
        }
        (to_allocate -> p_s).hi = 0;
        (to_allocate -> p_s).lo = 0;

        
        to_allocate -> p_time = 0;

        INIT_LIST_HEAD(&(to_allocate -> msg_inbox));

        to_allocate -> p_pid = next_pid++;

        return to_allocate;
    }
}

/*inizializza una variabile per essere una coda di processi*/
void mkEmptyProcQ(struct list_head *head) {
    INIT_LIST_HEAD(head);
}

/*ritorna TRUE se la lista head è vuota, ritorna FALSE altrimenti*/
int emptyProcQ(struct list_head *head) {
    return list_empty(head);
}

/*inserisci il pcb puntato da p nella coda di processi puntata da head*/
void insertProcQ(struct list_head *head, pcb_t *p) {
    list_add_tail(&p->p_list, head);
}

/*ritorna un puntatore al primo elemento della lista puntata da head, 
ritorna NULL se la lista è vuota*/
pcb_t *headProcQ(struct list_head *head) {
    if (emptyProcQ(head)) return NULL;
    else return container_of(list_next(head), pcb_t, p_list);
}

/*rimuove il primo elemento della coda di processi puntata da head e ritorna un puntatore
all'elemento rimosso, ritorna NULL se la coda è vuota*/
pcb_t *removeProcQ(struct list_head *head) {
    if (list_empty(head)){ 
        return NULL;
    }
    
    pcb_t *point = container_of(head->next , pcb_t, p_list);
    list_del(head->next);
    return point;
}

/*rimuovi il processo puntato da p dalla lista di processi puntata da head, se l'elemento
non è presente nella lista ritorna NULL, altrimenti ritorna p*/
pcb_t *outProcQ(struct list_head *head, pcb_t *p) {
    pcb_t *pholder;
    list_for_each_entry(pholder, head, p_list){
        if (pholder==p){
            list_del( &p->p_list);
            return(p);
        }
    }
    return(NULL);
}

/*ritorna TRUE se il processo puntato da p non ha figli, ritorna FALSE altrimenti*/
int emptyChild(pcb_t *p) {
    return list_empty(&p->p_child);
}

/*rendi il processo puntato da p un figlio del processo puntato da prnt*/
void insertChild(pcb_t *prnt, pcb_t *p) {
    struct list_head *frstchild = &prnt->p_child;
    struct list_head *sib = &p->p_sib;
    list_add_tail(sib, frstchild);
    p->p_parent=prnt;

}


/*rimuovi il primo elemento dalla lista dei figli di p e ritorna un puntatore all'elemento rimosso,
ritorna NULL se p non ha figli*/
pcb_t *removeChild(pcb_t *p) {

    if (list_empty(&p->p_child)){
        return NULL;
    }

    pcb_t *child = container_of((&p->p_child)->next, pcb_t, p_sib);
    (p->p_child).next = (child->p_sib).next;
    list_del(&child->p_sib);
    INIT_LIST_HEAD(&child->p_sib);
    child->p_parent = NULL;

    return child;

}

/*rimuovi il processo puntato da p dalla lista dei figli del proprio padre e ritorna p,
se p non ha un processo padre ritorna NULL*/
pcb_t *outChild(pcb_t *p) {

    if (p->p_parent == NULL){
        return NULL;
    }
    struct list_head *sib_list = &p->p_sib;
    struct list_head *next_sib = list_next(sib_list);
    list_del(sib_list);
    if (((p->p_parent)->p_child).next == sib_list){
        ((p->p_parent)->p_child).next = next_sib;
    }
    p->p_parent = NULL;
    INIT_LIST_HEAD(sib_list);
    return p;
}
