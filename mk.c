#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ALPHABET_SIZE 26
#define ALPHABET "abcdefghijklmnopqrstuvwxyz"

void free_val(void *value)
{
	free(value);
}

//*Trie lab11
typedef struct trie_node_t trie_node_t;
struct trie_node_t {
	void *value;	 /* Value is frequency of a word */
	int end_of_word; /* 1 if current node marks the end
	 of a word, 0 otherwise */
	trie_node_t **children;
	int n_children;
};

typedef struct trie_t trie_t;
struct trie_t {
	trie_node_t *root;

	/* Number of keys */
	int size;

	/* Generic Data Structure */
	int data_size;

	/* Trie-Specific, alphabet properties */
	int alphabet_size;
	char *alphabet;

	/* Callback to free value associated with key,
	should be called when freeing */
	void (*free_value_cb)(void *arg);

	/* Optional - number of nodes, useful to test correctness */
	int nr_nodes;
};

trie_node_t *trie_create_node(trie_t *trie)
{
	//aloc memorie pt un nou nod
	trie_node_t *node = calloc(1, sizeof(trie_node_t));
	//initializez campul end_of_word cu 0
	node->end_of_word = 0;
	//initializez campul n_children cu 0
	node->n_children = 0;
	//aloc memorie pt vectorul de copii ai nodului, de dimensiune alphabet_size
	node->children = calloc(trie->alphabet_size, sizeof(trie_node_t *));
	int i;
	for (i = 0; i < trie->alphabet_size; i++)
	//Initializez fiecare element din vectorul de copii cu NULL
		node->children[i] = NULL;
	//Returnez noul nod creat
	return node;
}

trie_t *trie_create(int data_size, int alphabet_size,
					char *alphabet, void (*free_value_cb)(void *))
{
	trie_t *trie = malloc(sizeof(trie_t));
	trie->data_size = data_size;
	trie->alphabet_size = alphabet_size;
	trie->alphabet = malloc(alphabet_size * sizeof(char));
	memcpy(trie->alphabet, alphabet, alphabet_size * sizeof(char));
	trie->free_value_cb = free_value_cb;
	trie->size = 1;
	trie->nr_nodes = 1;
	trie->root = trie_create_node(trie);
	trie->root->value = malloc(data_size);
	int dummy = -1;
	memcpy(trie->root->value, &dummy, data_size);
	trie->root->end_of_word = 0;
	trie->root->n_children = 0;
	return trie;
}

void trie_insert(trie_t *trie, char *key)
{
	//initializez un pointer catre radacina trie ului
	trie_node_t *aux = trie->root;
	//parcurg fiecare caracter din cheia data
	for (unsigned int i = 0; i < strlen(key); i++) {
		//verific daca copilul corespunzator caracterului este null
		if (aux->children[key[i] - 'a'] == NULL) {
			//daca da creez un nou nod p0t caracterul respectiv
			aux->children[key[i] - 'a'] = trie_create_node(trie);
			//incrementez nr de copii al nodului curent
			(aux->n_children)++;
			//incrementez nr total de noduri din trie
			(trie->nr_nodes)++;
		}
		//avansez catre urmatorul nod in arbore
		aux = aux->children[key[i] - 'a'];
	}
	//verific daca nodul curent este deja marcat ca sfarsit de cuvant
	if (aux->end_of_word == 0)
	//daca nu , aloc memorie pt val asociate acestuia
		aux->value = calloc(1, trie->data_size);

	int freq = 0;
	memcpy(&freq, aux->value, trie->data_size);
	freq++;
	memmove(aux->value, &freq, trie->data_size);
	//marchez nodul curent ca sfarsit de cuvant
	aux->end_of_word = 1;
}

void *trie_search(trie_t *trie, char *key)
{
	// TODO
	//initializez un pointer auxiliar catre radacina trie ului
	trie_node_t *aux = trie->root;
	//parcurg fiecare caracter
	for (unsigned int i = 0; i < strlen(key); i++) {
		//verific daca copilul corespunzator caracterului este null
		if (aux->children[key[i] - 'a'] == NULL)
		//daca da returnez null
			return NULL;
		aux = aux->children[key[i] - 'a'];
	}

	if (aux->end_of_word == 0)
		return NULL;
	//returnez val asociata nodului curent
	return aux->value;
}

int rec_remove(trie_node_t *node, trie_t *trie, char *key)
{
	if (*key == '\0') {
		//verific daca am ajuns la sfarsitul cheii
		if (node->end_of_word == 1) {
			//verific daca nodul curent este marcat ca sfarsit decuvant
			trie->free_value_cb(node->value);
			//eliberez valoarea asociata nodului
			//scad dimensiunea trie ului
			(trie->size)--;
			node->end_of_word = 0;

			if (node->n_children == 0)
			//verific daca nodul curent nu are copii si returnez 1 in acest caz
				return 1;
			else
				return 0;
		}
		return 0;
	}
	if (node->children[*key - 'a'] &&
		rec_remove(node->children[*key - 'a'], trie, key + 1)) {
	//verific daca exista un copil pt caracterul curent din cheie
	//si recursiv apelez functia pt acel copil
		free(node->children[*key - 'a']->children);
		//eliberez memoria pt vectorul de copii si nodul copil
		free(node->children[*key - 'a']);
		//scadem nr total de noduri din trie si nr de copii al nodului curent
		(trie->nr_nodes)--;
		(node->n_children)--;
		node->children[*key - 'a'] = NULL;
		if (node->n_children == 0 && node->end_of_word == 0)
			return 1;
	}
	return 0;
}

void trie_remove(trie_t *trie, char *key)
{
	//verific daca cheia exista in trie
	if (trie_search(trie, key))
	//apelez functia recursiva rec_remove
		rec_remove(trie->root, trie, key);
}

void rec_free(trie_node_t **node, trie_t *trie)
{
	for (int i = 0; i < trie->alphabet_size; i++) {
		if ((*node)->children[i])
			rec_free(&((*node)->children[i]), trie);
	}
	if ((*node)->end_of_word == 1)
	//eliberez valoarea asociata nodului
		trie->free_value_cb((*node)->value);
	//eliberez memoria pt vectorul de copii al nodului curent
	free((*node)->children);
	//eliberez memoria pt nodul curent
	free(*node);
}

void trie_free(trie_t **p_trie)
{
	//eliberez memoria pt valoarea asociata radacinii trie ului
	free((*p_trie)->root->value);
	//apelez functia de eliberare recursiva
	rec_free(&(*p_trie)->root, *p_trie);
	//eliberez memoria pt vectorul de caractere din alfabet
	free((*p_trie)->alphabet);
	//eliberez memoria pt trie ul insusi
	free(*p_trie);
}

// lab11

void rec_autocorrect(trie_node_t *node, char str[],
					 int level, char *word, int k, int *flag)
{
	if (node->n_children == 0 || node->end_of_word == 1) {
		//verific daca nodul curent este o frunza
		//sau nod final al unui cuvant
		size_t x = 0;
		size_t y = 0;

		int diff = 0;

		//Compar caracterele intre cuvantul corect si
		//cuvantul corect si cuvantul gresit pentru a
		//numara diferentele dintre ele
		while (x < strlen(word) && y < strlen(str)) {
			if (word[x] != str[y])
				diff++;
			x++;
			y++;
		}

		//Marchez sfarsitul sirului temporar
		str[level] = '\0';

		//verific daca deiferenta calculata este mai mica sau
		//mai mare decat nr de modificari admise(k) si daca
		//cuvintele au aceasi lungime
		if (diff <= k && strlen(word) == strlen(str)) {
			//setez un flag pentru a indica gasirea unui cuvant
			*flag = 1;
			printf("%s\n", str);
		}
	}

	int i;
	for (i = 0; i < ALPHABET_SIZE; ++i) {
	// parcurg toti copiii nodului curent
		if (node->children[i]) {
			// adaug caracterul corespunzator copilului in sir
			str[level] = i + 'a';
			//Parcurgem recursiv pt a explora trie-ul
			rec_autocorrect(node->children[i], str, level + 1, word, k, flag);
		}
	}
}

void rec_autocomplete_1(trie_node_t *node, char *str,
						int level, char *prefix, char *word, int *flag)
{
	if (node->n_children == 0 || node->end_of_word == 1) {
		//verific daca nodul curent este frunza
		//sau este nod final de cuvant
		str[level] = '\0';
		if (strncmp(prefix, str, strlen(prefix)) == 0) {
			//verific daca cuvantul format in la pasul curent
			//se potriveste cu prefixul specificat
			if (*flag == 0) {
			//verific daca nu s a gasit inca niciun cuvant valid
				strcpy(word, str);
				*flag = 1;
			} else {
				//daca s a gasit un cuvant valid
				if (strcmp(str, word) < 0)
				//verific daca cuv gasit este mai mic lexicografic
				//decat cel stocat inn word
					strcpy(word, str);
				//actualizez cuvantul stocat in word cu cuv curent
			}
		}
	}

	int i;
	for (i = 0; i < ALPHABET_SIZE; ++i) {
	// parcurg toti copiii nodului curent
		if (node->children[i]) {
			// adaug caracterul corespunzator copilului in sir
			str[level] = i + 'a';
			rec_autocomplete_1(node->children[i], str,
							   level + 1, prefix, word, flag);
			//apel recursiv pt a parcurge trie ul
		}
	}
}

void rec_autocomplete_2(trie_node_t *node, char *str,
						int level, char *prefix, char *word, int *flag)
{
	if (node->n_children == 0 || node->end_of_word == 1) {
		//daca nodul nu are copii sau reprezinta sf unui cuv
		str[level] = '\0';
		//adaug terminatorul de sir
		if (strncmp(prefix, str, strlen(prefix)) == 0) {
		//verific daca prefixul este unul valid
			if (*flag == 0) {
		//daca este primul cuvant gasit copiaza intregul sir in word
				strcpy(word, str);
				*flag = 1;
			} else {
		//daca este un cuvant gasit ulterior , compar lungimea cuvintelor
		//si copiez cuvantul mai scurt in word
				if (strlen(str) < strlen(word))
					strcpy(word, str);
			}
		}
	}

	int i;
	for (i = 0; i < ALPHABET_SIZE; ++i) {
	// parcurg toti copiii nodului curent
		if (node->children[i]) {
			// adaug caracterul corespunzator copilului in sir
			str[level] = i + 'a';
			rec_autocomplete_2(node->children[i], str,
							   level + 1, prefix, word, flag);
		}
	}
}

void rec_autocomplete_3(trie_node_t *node, char str[], int level,
						char *prefix, char *word, int *flag, int *freq)
{
	if (node->n_children == 0 || node->end_of_word == 1) {
		//daca este node frunza sau este finalul cuvantului
		str[level] = '\0';
		//adaug terminatorul de sfarsit de sir
		if (strncmp(prefix, str, strlen(prefix)) == 0) {
		//verific daca prefixul este unul valid
			if (*flag == 0) {
			//daca este primul cuv il copiez in word
				strcpy(word, str);
				*flag = 1;

				memcpy(freq, node->value, sizeof(int));
			} else {
		//daca este un cuvant gasit ulterior compar frecventa
		//si cuv si actualizez word si freq
				int aux_freq = 0;
				memcpy(&aux_freq, node->value, sizeof(int));

				if (aux_freq > *freq) {
					strcpy(word, str);
					*freq = aux_freq;
				} else if (aux_freq == *freq) {
					if (strcmp(str, word) < 0)
						strcpy(word, str);
				}
			}
		}
	}

	int i;
	for (i = 0; i < ALPHABET_SIZE; ++i) {
	// parcurg toti copiii nodului curent
		if (node->children[i]) {
			// adaug caracterul corespunzator copilului in sir
			str[level] = i + 'a';
			rec_autocomplete_3(node->children[i], str, level + 1,
							   prefix, word, flag, freq);
		}
	}
}

void load(trie_t *trie, char *filename)
{
	//deschid fisierul pt citire
	FILE *file = fopen(filename, "r");
	//verific daca fisierul exista si poate fi deschis
	if (!file) {
		printf("File not found\n");
		return;
	}
	//declar un buffer pt citirea cuvintelor din fisier
	char buffer[100];
	//parcurg fisierul si citesc cuv pana la sfarsitul acestuia
	while (fscanf(file, "%s", buffer) != EOF)
	//inserez cuv in trie
		trie_insert(trie, buffer);
	//inchid fisierul
	fclose(file);
}

void autocorrect(trie_t *trie, char *input_buffer, int k)
{
	int level = 0;
	char str[100] = {0};
	int flag = 0;
	//apelez functia de autocorectare recursiva pt radacina trie ului
	rec_autocorrect(trie->root, str, level, input_buffer, k, &flag);
	//verific daca nu s au gasit cuvinte
	if (flag == 0)
		printf("No words found\n");
}

void autocomplete(trie_t *trie, char *input_buffer, int nr_criteriu)
{
	//aloc memorie
	char *str = calloc(100, sizeof(char));
	char *return_word = calloc(100, sizeof(char));

	int flag = 0;
	int level = 0;

	//verific nr criteriului pt autocomplete
	if (nr_criteriu == 0) {
		//apelez fct de autocomplete1
		rec_autocomplete_1(trie->root, str, level,
						   input_buffer, return_word, &flag);
		if (flag == 0)
			printf("No words found\n");
		else
			printf("%s\n", return_word);

		memset(str, 0, 100 * sizeof(char));
		memset(return_word, 0, 100 * sizeof(char));

		flag = 0;
		//apelez functia de autocomplete2
		rec_autocomplete_2(trie->root, str, level,
						   input_buffer, return_word, &flag);
		if (flag == 0)
			printf("No words found\n");
		else
			printf("%s\n", return_word);

		//eliberez memoria si reinitializez variabilele
		memset(str, 0, 100 * sizeof(char));
		memset(return_word, 0, 100 * sizeof(char));

		int freq = 0;
		flag = 0;
		//apelez functia de autocomplete3
		rec_autocomplete_3(trie->root, str, level,
						   input_buffer, return_word, &flag, &freq);
		if (flag == 0)
		//verific rezultatul functiei si afisez cuvantul
		//returnat sau un mesaj de eroare
			printf("No words found\n");
		else
			printf("%s\n", return_word);
	} else {
		//verific nr criteriului si apelez functia respectiva
		if (nr_criteriu == 1) {
			rec_autocomplete_1(trie->root, str, level,
							   input_buffer, return_word, &flag);
		} else if (nr_criteriu == 2) {
			rec_autocomplete_2(trie->root, str, level,
							   input_buffer, return_word, &flag);
		} else if (nr_criteriu == 3) {
			int freq = 0;
			rec_autocomplete_3(trie->root, str, level,
							   input_buffer, return_word, &flag, &freq);
		}

		if (flag == 0)
			printf("No words found\n");
		else
			printf("%s\n", return_word);
	}

	free(str);
	free(return_word);
}

int main(void)
{
	char command[20], input_buffer[20];
	int k, nr_criteriu;
	char alphabet[] = ALPHABET;
	trie_t *trie = trie_create(sizeof(int), ALPHABET_SIZE, alphabet, &free_val);
	while (1) {
		scanf("%s", command);
		if (strcmp(command, "INSERT") == 0) {
			scanf("%s", input_buffer);
			trie_insert(trie, input_buffer);
		} else if (strcmp(command, "LOAD") == 0) {
			char filename[100];
			scanf("%s", filename);
			load(trie, filename);
		} else if (strcmp(command, "REMOVE") == 0) {
			scanf("%s", input_buffer);
			trie_remove(trie, input_buffer);
		} else if (strcmp(command, "AUTOCORRECT") == 0) {
			scanf("%s %d", input_buffer, &k);
			autocorrect(trie, input_buffer, k);
		} else if (strcmp(command, "AUTOCOMPLETE") == 0) {
			scanf("%s %d", input_buffer, &nr_criteriu);
			autocomplete(trie, input_buffer, nr_criteriu);
		} else if (strcmp(command, "EXIT") == 0) {
			trie_free(&trie);
			break;
		}
	}
	return 0;
}
