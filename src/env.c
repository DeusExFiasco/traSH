#include "trash.h"

char **dup_env(char **envp) {
    int count = 0;
    while (envp[count])
        count++;
    char **copy = malloc(sizeof(char *) * (count + 1));
    if (!copy)
        return NULL;
    for (int i = 0; i < count; ++i) {
        copy[i] = strdup(envp[i]);
        if (!copy[i]) {
            for (int j = 0; j <= i; j++)
                free(copy[j]);
            free(copy);
            return NULL;
        }
    }
    copy[count] = NULL;
    return copy;
}

bool is_valid_identifier(const char *str) {
    if (!str || !((*str == '_') || isalpha((unsigned char)*str)))
        return false;
    for (str++; *str; str++) {
        if (!((*str == '_') || isalpha((unsigned char)*str)))
            return false;
    }
    return true;
}

char *get_env_var(char *name, char **env) {
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

bool set_env(char ***envp, const char *name, const char *value) {
    size_t len_name = strlen(name);
    size_t len_val = strlen(value);
    char *key_value = malloc(len_name + 1 + len_val + 1);
    if (!key_value)
        return false;
    memcpy(key_value, name, len_name);
    key_value[len_name] = '=';
    memcpy(key_value + len_name + 1, value, len_val);
    key_value[len_name + 1 +len_val] = '\0';

    char **env = *envp;
    for (int i = 0; env && env[i]; ++i) {
        if (strncmp(env[i], name, len_name) == 0 && env[i][len_name] == '=') {
            free(env[i]);
            env[i] = key_value;
            return true;
        }
    }

    int count = 0;
    while (env && env[count])
        count++;
    char **new_env = realloc(env, sizeof(char *) * (count + 2));
    if (!new_env) {
        free(key_value);
        return false;
    }
    new_env[count] = key_value;
    new_env[count + 1] = NULL;
    *envp = new_env;
    return true;
}

int unset_env(char ***envp, const char *name) {
    char **env = *envp;
    size_t len = strlen(name);
    for (int i = 0; env && env[i]; ++i) {
        if (strncmp(env[i], name, len) == 0 && env[i][len] == '=') {
            free(env[i]);
            for (int j = i; env[j]; ++j)
                env[j] = env[j + 1];
            return true;
        }
    }
    return false;
}

bool append_env(char ***envp, const char *name, const char *suffix) {
    const char *old = get_env_var(strdup(name), *envp);
    size_t len = strlen(old) + strlen(suffix);
    char *merged = malloc(len + 1);
    if (!merged)
        return false;
    memcpy(merged, old, strlen(old));
    memcpy(merged + strlen(old), suffix, strlen(suffix));
    merged[len] = '\0';
    bool ok = set_env(envp, name, merged);
    free(merged);
    return ok;
}
