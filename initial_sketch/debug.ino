//
// debug logger
//
#define FAST_FORWARD_BUFFER_SIZE 50

//
// very simple debug logger
//
// example:
//
//   call  : debug("pin: %d, high: %b", 4, isHigh());
//   output: pin: 4, high: true
//
//
// supported placeholder:
//
//   %d: integer
//   %f: double
//   %s: string
//   %b: boolean
//   %%: %
//
void debug(const char* fmt, ...) {
  if(SERIAL_DEBUGGING_ENABLED) {
    char ffb[FAST_FORWARD_BUFFER_SIZE];
    int ffb_idx = 0;

    va_list args;
    va_start(args, fmt);
    for(; *fmt != 0; fmt++) {

      // fast forward to the next format symbol
      //
      //  - 'Serial.print' is slow
      //  - output was interleaved sometimes
      //  - buffer the text and print them from the buffer
      //    if the buffer is full / all text is consumed
      ffb_idx = 0;
      while(*fmt != '%' && *fmt != 0) {
        ffb[ffb_idx++] = *fmt;
        fmt++;

        // buffer full - flush the buffer
        if(ffb_idx == FAST_FORWARD_BUFFER_SIZE - 1) {
          ffb[ffb_idx] = 0;
          Serial.print(ffb);
          ffb_idx = 0;
        }

        if(*fmt == 0) {
          break;
        }
      }
      // flush the buffer
      ffb[ffb_idx] = 0;
      Serial.print(ffb);

      if(*fmt == 0) {
        break;
      }


      // skip the format symbol escape sequence '%'
      fmt++;

      // interpret the current format symbol
      switch(*fmt) {
      case 'd':
        Serial.print(va_arg(args, int));
        break;
      case 'f':
        Serial.print(va_arg(args, double));
        break;
      case 's':
        Serial.print(va_arg(args, char *));
        break;
      case 'b':
        Serial.print(va_arg(args, int) ? "true" : "false");
        break;
      case '%':
        Serial.print("%");
        break;
      default:
        Serial.print("ERROR: invalid format specifier: ");
        Serial.println(*fmt);
      }
    }
    va_end(args);
    Serial.println();
  }
}
