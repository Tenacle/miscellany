#ifndef SAFEREADER_H
#define SAFEREADER_H

FILE *openFile(char *fileName, char *type);
int tokenizeAsHex(uint8_t *hex, size_t size, FILE *file);
//int setDelimiter(char *delimiter);

#endif
