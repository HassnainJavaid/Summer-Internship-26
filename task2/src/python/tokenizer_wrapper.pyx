# cython: language_level=3
from libc.stdlib cimport free, malloc
from libc.stddef cimport size_t

cdef extern from "../../include/c_tokenizer.h":
    ctypedef struct TokenizerData:
        pass

    TokenizerData* tokenizer_init(const char* vocab_path, const char* merges_path)
    void tokenizer_free(TokenizerData* data)
    int* tokenizer_encode(const TokenizerData* data, const char* text, size_t* out_len)
    char* tokenizer_decode(const TokenizerData* data, const int* ids, size_t len)

cdef class PyTokenizer:
    cdef TokenizerData* _handle

    def __cinit__(self, str vocab_path, str merges_path):
        self._handle = tokenizer_init(vocab_path.encode('utf-8'), merges_path.encode('utf-8'))
        if self._handle == NULL:
            raise MemoryError("Failed to initialize C Tokenizer.")

    def __dealloc__(self):
        if self._handle != NULL:
            tokenizer_free(self._handle)

    def encode(self, str text) -> list[int]:
        cdef size_t out_len = 0
        cdef bytes encoded_bytes = text.encode('utf-8')
        cdef int* ptr = tokenizer_encode(self._handle, encoded_bytes, &out_len)
        
        if ptr == NULL:
            return []

        cdef list py_list = [ptr[i] for i in range(out_len)]
        free(ptr)
        return py_list

    def decode(self, list ids) -> str:
        if not ids:
            return ""

        # Declare all cdef variables up front before try/except blocks
        cdef size_t length = len(ids)
        cdef int* c_arr = NULL
        cdef char* raw_str = NULL
        cdef str py_str = ""
        cdef size_t i

        c_arr = <int*>malloc(length * sizeof(int))
        if c_arr == NULL:
            raise MemoryError("Failed memory allocation for input token IDs.")

        try:
            for i in range(length):
                c_arr[i] = ids[i]

            raw_str = tokenizer_decode(self._handle, c_arr, length)
            if raw_str == NULL:
                return ""

            py_str = raw_str.decode('utf-8', errors='replace')
            free(raw_str)
            return py_str
        finally:
            free(c_arr)