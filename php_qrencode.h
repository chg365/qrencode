/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_QRENCODE_H
#define PHP_QRENCODE_H

extern zend_module_entry qrencode_module_entry;
#define phpext_qrencode_ptr &qrencode_module_entry

#define PHP_QRENCODE_EXTNAME "qrencode"
#define PHP_QRENCODE_VERSION "0.2.0" /* Replace with version number for your extension */
#define LE_QRENCODE "qrencode handle"

#ifdef PHP_WIN32
#    define PHP_QRENCODE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_QRENCODE_API __attribute__ ((visibility("default")))
#else
#    define PHP_QRENCODE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(qrencode);
PHP_MSHUTDOWN_FUNCTION(qrencode);
PHP_MINFO_FUNCTION(qrencode);
PHP_FUNCTION(qr_encode);
PHP_FUNCTION(qr_version);
PHP_FUNCTION(qr_save);

/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:

ZEND_BEGIN_MODULE_GLOBALS(qrencode)
	zend_long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(qrencode)
*/

/* Always refer to the globals in your function as QRENCODE_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define QRENCODE_G(v) TSRMG(qrencode_globals_id, zend_qrencode_globals *, v)
# ifdef COMPILE_DL_QRENCODE
ZEND_TSRMLS_CACHE_EXTERN()
# endif
#else
#define QRENCODE_G(v) (qrencode_globals.v)
/*#define QRENCODE_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(qrencode, v)*/
#endif

#endif	/* PHP_QRENCODE_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
