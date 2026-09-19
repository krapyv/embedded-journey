/* UART2 CONFIGURATION */
#define UART_MODE_TX_ONLY 0U
#define UART_MODE_RX_ONLY 1U
#define UART_MODE_TX_RX 2U

// select active mode for the project
#define TARGET_UART_MODE UART_MODE_TX_RX

typedef enum
{
    TOKEN_OPERAND = 0,
    TOKEN_OP_ADD,
    TOKEN_OP_SUB,
    TOKEN_OP_MULT,
    TOKEN_OP_DIV,
    TOKEN_INVALID
} TOKEN_Classification_t;