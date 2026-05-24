#include "minishell.h"

static bool is_operator(const char c) {
    return (
        c == '|' ||
        c == '&' ||
        c == '<' ||
        c == '>' ||
        c == ';'
    );
}

static token_t *create_token(const token_type_t type, char *value) {
    if (!value && type != TK_EOF)
        return nullptr;
    token_t* token = malloc(sizeof(token_t));
    if (!token)
        return nullptr;
    token->type = type;
    token->value = value;
    token->next = nullptr;

    return token;
}

static void add_token(token_t **list, token_t *new_token) {
    if (!*list) {
        *list = new_token;
        return;
    }
    token_t *temp = *list;
    while (temp->next)
        temp = temp->next;
    temp->next = new_token;
}

static token_t *read_operator(const char *input, int *i) {
    if (input[*i] == '|' && input[*i + 1] == '|') {
        *i += 2;
        return(create_token(TK_OR, strdup("||")));
    }
    if (input[*i] == '&' && input[*i + 1] == '&') {
        *i += 2;
        return(create_token(TK_AND, strdup("&&")));
    }
    if (input[*i] == '>' && input[*i + 1] == '>') {
        *i += 2;
        return(create_token(TK_APPEND, strdup(">>")));
    }
    if (input[*i] == '<' && input[*i + 1] == '<') {
        *i += 2;
        return(create_token(TK_HEREDOC, strdup("<<")));
    }
    if (input[*i] == '|') {
        (*i)++;
        return(create_token(TK_PIPE, strdup("|")));
    }
    if (input[*i] == '>') {
        (*i)++;
        return(create_token(TK_REDIR_OUT, strdup(">")));
    }
    if (input[*i] == '<') {
        (*i)++;
        return(create_token(TK_REDIR_IN, strdup("<")));
    }
    if (input[*i] == ';') {
        (*i)++;
        return(create_token(TK_SEMICOLON, strdup(";")));
    }
    return nullptr;
}

static char *extract_word(const char *input, int *i) {
    const int start = *i;

    while (input[*i]) {
        if (input[*i] == '\'' || input[*i] == '"') {
            const char quote = input[*i];
            (*i)++;
            while (input[*i] && input[*i] != quote)
                (*i)++;
            if (input[*i] == quote)
                (*i)++;
        } else if (is_operator(input[*i]) || input[*i] == ' ')
            break;
        else
            (*i)++;
    }
    return substr(input, start, *i - start);
}

static char *get_env_var(char *name, char **env) {
    int i = 0;
    const size_t len = strlen(name);
    while (env[i]) {
        if (strncmp(env[i], name, len) == 0 && env[i][len] == '=') {
            char *value = strdup(env[i] + len + 1);
            free(name);
            return value;
        }
        i++;
    }
    free(name);
    return strdup("");
}

static char *expand_variable(const char *word, int *i, const shell_t *shell) {
    (*i)++;
    if (word[*i] == '?') {
        (*i)++;
        return itoa(shell->last_status);
    }
    int var_len = 0;
    const int start = *i;
    while (word[*i] && (isalnum(word[*i]) || word[*i] == '_')) {
        var_len++;
        (*i)++;
    }
    char *var_name = malloc(var_len + 1);
    if (!var_name)
        return nullptr;
    *i = start;
    int j = 0;
    while (word[*i] && (isalnum(word[*i]) || word[*i] == '_')) {
        var_name[j++] = word[*i];
        (*i)++;
    }
    var_name[j] = '\0';
    return(get_env_var(var_name, shell->env));
}

static void update_quote_state(const char c, quote_state_t *state) {
    if (c == '\'' && *state == QUOTE_NONE)
        *state = QUOTE_SINGLE;
    else if (c == '"' && *state == QUOTE_NONE)
        *state = QUOTE_DOUBLE;
    else if ((c == '"' && *state == QUOTE_DOUBLE) || (c == '\'' && *state == QUOTE_SINGLE))
        *state = QUOTE_NONE;
}

static char *process_word(const char *raw, const shell_t *shell) {
    char *result = nullptr;
    quote_state_t state = QUOTE_NONE;
    int i = 0;

    while (raw[i]) {
        if ((raw[i] == '\'' && state != QUOTE_DOUBLE) || (raw[i] == '"' && state != QUOTE_SINGLE)) {
            update_quote_state(raw[i], &state);
            i++;
        } else if (raw[i] == '$' && state != QUOTE_SINGLE) {
            char *expanded = expand_variable(raw, &i, shell);
            if (expanded) {
                result = append_str(result, expanded);
                free(expanded);
            }
        } else {
            result = append_char(result, raw[i]);
            i++;
        }
    }
    return result;
}

token_t *tokenize_input(shell_t *shell) {
    token_t *tokens = nullptr;
    token_t *new_token;
    const char *input = shell->input;
    int i = 0;

    while (input[i]) {
        while (input[i] == ' ')
            i++;
        if (!input[i])
            break;
        if (is_operator(input[i]))
            new_token = read_operator(input, &i);
        else {
            const char *raw_word = extract_word(input, &i);
            if (!raw_word)
                handle_fatal_error(MEMORY_ERROR, nullptr, shell);
            new_token = create_token(TK_WORD, process_word(raw_word, shell));
        }
        if (!new_token)
            handle_fatal_error(MEMORY_ERROR, nullptr, shell);
        add_token(&tokens, new_token);
    }
    add_token(&tokens, create_token(TK_EOF, nullptr));
    return tokens;
}
