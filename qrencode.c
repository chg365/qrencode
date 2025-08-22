/*
  +----------------------------------------------------------------------+
  | PHP Version 8                                                        |
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <png.h>
#include <qrencode.h>
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_qrencode.h"
#include "php_open_temporary_file.h"

/* If you declare any globals in php_qrencode.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(qrencode)
*/

/* True global resources - no need for thread safety here */
static int le_qr;

// 统一 php_qrcode 结构体定义
#if PHP_VERSION_ID >= 80000
struct _php_qrcode {
    QRcode *c;
    zend_object std;
};

// 辅助函数：从对象获取 php_qrcode
static inline php_qrcode *php_qrcode_from_obj(zend_object *obj)
{
    return (php_qrcode *)((char *)(obj) - XtOffsetOf(struct _php_qrcode, std));
}
#else
typedef struct {
    QRcode *c;
} php_qrcode;
#endif

/* PHP 版本兼容性处理 */
#if PHP_VERSION_ID >= 80000
// PHP 8.x 使用对象而不是资源
#define QR_RESOURCE_NAME "QR Code"
zend_class_entry *qrencode_class_entry;
static zend_object_handlers qrencode_object_handlers;
#endif

// 添加 arginfo 定义
ZEND_BEGIN_ARG_INFO_EX(arginfo_qr_encode, 0, 0, 1)
    ZEND_ARG_INFO(0, text)
    ZEND_ARG_INFO(0, version)
    ZEND_ARG_INFO(0, level)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, casesensitive)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_qr_version, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_qr_save, 0, 0, 1)
#if PHP_VERSION_ID >= 80000
    ZEND_ARG_OBJ_INFO(0, link, QRCode, 0)
#else
    ZEND_ARG_INFO(0, link)
#endif
    ZEND_ARG_INFO(0, filename)
    ZEND_ARG_INFO(0, size)
    ZEND_ARG_INFO(0, margin)
ZEND_END_ARG_INFO()

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("qrencode.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_qrencode_globals, qrencode_globals)
    STD_PHP_INI_ENTRY("qrencode.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_qrencode_globals, qrencode_globals)
PHP_INI_END()
*/
/* }}} */

#if PHP_VERSION_ID >= 80000
// PHP 8.x 对象处理函数
static void qrencode_free_object(zend_object *object)
{
    php_qrcode *qr = php_qrcode_from_obj(object);
    
    if (qr->c)
        QRcode_free(qr->c);
    
    zend_object_std_dtor(&qr->std);
}

static zend_object *qrencode_create_object(zend_class_entry *class_type)
{
    php_qrcode *qr = (php_qrcode *)ecalloc(1, sizeof(php_qrcode));
    
    zend_object_std_init(&qr->std, class_type);
    object_properties_init(&qr->std, class_type);
    
    qr->std.handlers = &qrencode_object_handlers;
    
    return &qr->std;
}
#endif

/* {{{ resource qr_encode (string text, [int version, int level, int mode, int casesensitive]); */
/**
 * @brief main function to encode text to qr code.
 * @param text the string want to encode.
 * @param version qrcode version
 * @param level level
 * @param mode mode, could be QR_MODE_NUM, QR_MODE_AN, QR_MODE_8, QR_MODE_KANJI.
 * @param casesensitive casesentive, if you want the 8 bit mode, must set this on.
 * @return
 */
PHP_FUNCTION(qr_encode)
{
    php_qrcode *qr = NULL;
    long version = 1, level = QR_ECLEVEL_L, mode = QR_MODE_8, casesensitive = 1;
    const char *text;
    size_t text_len;

    if (zend_parse_parameters( ZEND_NUM_ARGS(), "s|llll", &text, &text_len, &version, &level, &mode, &casesensitive) == FAILURE)
        RETURN_FALSE;

    qr = (php_qrcode *) emalloc (sizeof (php_qrcode));
    if (mode == QR_MODE_8)
        qr->c = QRcode_encodeString8bit(text, version, level);
    else
        qr->c = QRcode_encodeString(text, version, level, mode, casesensitive);

    if (qr->c == NULL)
    {
        efree (qr);
        RETURN_FALSE;
    }

#if PHP_VERSION_ID >= 80000
    // PHP 8.x 使用对象
    zend_object *obj = qrencode_create_object(qrencode_class_entry);
    php_qrcode *obj_qr = php_qrcode_from_obj(obj);
    obj_qr->c = qr->c;
    efree(qr); // 释放临时分配的内存
    
    RETURN_OBJ(obj);
#else
    // PHP 7.x 及以下使用资源
    #if PHP_VERSION_ID >= 70000
        RETURN_RES(zend_register_resource(qr, le_qr));
    #else
        ZEND_REGISTER_RESOURCE (return_value, qr, le_qr);
    #endif
#endif
}
/* }}} */
/* qr_version {{{ */
PHP_FUNCTION(qr_version)
{
#if PHP_VERSION_ID >= 70000
    RETURN_STRING(PHP_QRENCODE_VERSION);
#else
	RETURN_STRING(PHP_QRENCODE_VERSION, 1);
#endif
}
/* }}} */
/* {{{ int qr_save (resource link, [string filename, int size, int margin]); */
/**
 * @brief save function for qrencode.
 * @param link qrcode resource link.
 * @param filename filename want to save, it could be empty, and will show the content directly.
 * @param size size
 * @param margin margin.
 * @return
 */
PHP_FUNCTION(qr_save)
{
    zval *link = NULL;
    long size = 3, margin = 4;
    const char *fn = NULL;
    int argc;
    size_t fn_len;
    FILE *fp = NULL;
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned char *row, *p, *q;
    int x, y, xx, yy, bit;
    int realwidth;
#if PHP_VERSION_ID >= 70000
    zend_string *path;
#else
	char *path;
#endif
    int b;
    char buf[4096];

    argc = ZEND_NUM_ARGS();

    // 修改参数解析以支持 PHP 8.x 的对象
#if PHP_VERSION_ID >= 80000
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "O|sll", &link, qrencode_class_entry, &fn, &fn_len, &size, &margin) == FAILURE)
#else
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r|sll", &link, &fn, &fn_len, &size, &margin) == FAILURE)
#endif
        RETURN_FALSE;

    if (link)
    {
        php_qrcode *qr = NULL;

#if PHP_VERSION_ID >= 80000
        // PHP 8.x 对象处理
        qr = php_qrcode_from_obj(Z_OBJ_P(link));
#else
        // PHP 7.x 及以下资源处理
        #if PHP_VERSION_ID >= 70000
            if ((qr = (php_qrcode *)zend_fetch_resource(Z_RES_P(link), LE_QRENCODE, le_qr)) == NULL) {
                RETURN_FALSE;
            }
        #else
            ZEND_FETCH_RESOURCE (qr, php_qrcode *, &link, -1, LE_QRENCODE, le_qr);
        #endif
#endif

        if (argc >= 2 && fn != NULL && strlen(fn)!=0)
        {
            fp = VCWD_FOPEN (fn, "wb");
            if (!fp)
            {
                php_error_docref(NULL, E_WARNING, "Unable to open '%s' for writing.", fn);
                RETURN_FALSE;
            }
        }
        else
        {
            fp = php_open_temporary_file (NULL, NULL, &path);
            if (!fp)
            {
                php_error_docref(NULL, E_WARNING, "Unable to open temporary file for writing.");
                RETURN_FALSE;
            }
        }

        realwidth = (qr->c->width + margin * 2) * size;
        row = (unsigned char *) emalloc ((realwidth + 7) / 8);

        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if(png_ptr == NULL)
        {
            fclose(fp);
            php_error_docref(NULL, E_WARNING, "Failed to initialize PNG writer.");
            RETURN_FALSE;
        }

        info_ptr = png_create_info_struct(png_ptr);
        if(info_ptr == NULL)
        {
            fclose(fp);
            php_error_docref(NULL, E_WARNING, "Failed to initialize PNG write.");
            RETURN_FALSE;
        }


        if(setjmp(png_jmpbuf(png_ptr))) {
            png_destroy_write_struct(&png_ptr, &info_ptr);
            fclose(fp);
            php_error_docref(NULL, E_WARNING, "Failed to write PNG image.");
            RETURN_FALSE;
        }

        png_init_io(png_ptr, fp);

        png_set_IHDR(png_ptr, info_ptr, realwidth, realwidth, 1, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);

        memset(row, 0xff, (realwidth + 7) / 8);
        for(y = 0; y < margin * size; y++)
            png_write_row(png_ptr, row);

        p = qr->c->data;
        for(y = 0; y < qr->c->width; y++)
        {
            bit = 7;
            memset(row, 0xff, (realwidth + 7) / 8);
            q = row;
            q += margin * size / 8;
            bit = 7 - (margin * size % 8);
            for(x = 0; x < qr->c->width; x++)
            {
                for(xx = 0; xx < size; xx++)
                {
                    *q ^= (*p & 1) << bit;
                    bit--;
                    if(bit < 0)
                    {
                        q++;
                        bit = 7;
                    }
                }
                p++;
            }

            for(yy=0; yy<size; yy++)
                png_write_row(png_ptr, row);
        }

        memset(row, 0xff, (realwidth + 7) / 8);
        for(y=0; y<margin * size; y++)
            png_write_row(png_ptr, row);

        png_write_end(png_ptr, info_ptr);
        png_destroy_write_struct(&png_ptr, &info_ptr);

        efree (row);

        if (argc >= 2 && fn != NULL && strlen(fn)!=0)
        {
            fflush (fp);
            fclose (fp);
        }
        else
        {
            fseek (fp, 0, SEEK_SET);
#if APACHE && defined(CHARSET_EBCDIC)
            ap_bsetflag(php3_rqst->connection->client, B_EBCDIC2ASCII, 0);
#endif
            while ((b = fread (buf, 1, sizeof(buf), fp)) > 0)
                php_write (buf, b);

            fclose (fp);

#if PHP_VERSION_ID >= 80000
            VCWD_UNLINK((const char *)ZSTR_VAL(path));
            zend_string_release(path);
#else
            #if PHP_VERSION_ID >= 70000
                VCWD_UNLINK((const char *)ZSTR_VAL(path));
                zend_string_release(path);
            #else
                VCWD_UNLINK((const char *)path);
            #endif
#endif
        }

        RETURN_TRUE;
    }
    else
        RETURN_FALSE;
}
/* }}} */

#if PHP_VERSION_ID >= 80000
// PHP 8.x 不需要资源析构函数
#else
// PHP 7.x 及以下需要资源析构函数
#if PHP_VERSION_ID >= 70000
static void qr_dtor(zend_resource *rsrc)
#else
static void qr_dtor(zend_rsrc_list_entry *rsrc)
#endif
{
    php_qrcode *qr = (php_qrcode *)rsrc->ptr;

    if (qr->c)
        QRcode_free (qr->c);
    efree (qr);
}
#endif

/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_qrencode_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_qrencode_init_globals(zend_qrencode_globals *qrencode_globals)
{
    qrencode_globals->global_value = 0;
    qrencode_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(qrencode)
{
    /* If you have INI entries, uncomment these lines
    REGISTER_INI_ENTRIES();
    */
    
#if PHP_VERSION_ID >= 80000
    // PHP 8.x 注册对象类
    zend_class_entry ce;
    
    INIT_CLASS_ENTRY(ce, "QRCode", NULL);
    qrencode_class_entry = zend_register_internal_class(&ce);
    qrencode_class_entry->create_object = qrencode_create_object;
    
    memcpy(&qrencode_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    qrencode_object_handlers.offset = XtOffsetOf(struct _php_qrcode, std);
    qrencode_object_handlers.free_obj = qrencode_free_object;
#else
    // PHP 7.x 及以下注册资源
    le_qr = zend_register_list_destructors_ex(qr_dtor, NULL, LE_QRENCODE, module_number);
#endif

    REGISTER_LONG_CONSTANT ("QR_MODE_NUL", QR_MODE_NUL, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("QR_MODE_NUM", QR_MODE_NUM, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("QR_MODE_AN", QR_MODE_AN, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("QR_MODE_8", QR_MODE_8, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("QR_MODE_KANJI", QR_MODE_KANJI, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("QR_ECLEVEL_L", QR_ECLEVEL_L, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("QR_ECLEVEL_M", QR_ECLEVEL_M, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("QR_ECLEVEL_Q", QR_ECLEVEL_Q, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT ("QR_ECLEVEL_H", QR_ECLEVEL_H, CONST_CS | CONST_PERSISTENT);
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(qrencode)
{
    /* uncomment this line if you have INI entries
    UNREGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
/*
PHP_RINIT_FUNCTION(qrencode)
{
#if defined(COMPILE_DL_QRENCODE) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    return SUCCESS;
}
*/
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
/*
PHP_RSHUTDOWN_FUNCTION(qrencode)
{
    return SUCCESS;
}
*/
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(qrencode)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "qrencode support", "enabled");
    php_info_print_table_header(2, "qrencode version", PHP_QRENCODE_VERSION);
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}
/* }}} */

/* {{{ qrencode_functions[]
 *
 * Every user visible function must have an entry in qrencode_functions[].
 */
const zend_function_entry qrencode_functions[] = {
    PHP_FE(qr_encode, arginfo_qr_encode)
    PHP_FE(qr_version, arginfo_qr_version)
    PHP_FE(qr_save, arginfo_qr_save)
    /*{NULL, NULL, NULL}*/
    PHP_FE_END  /* Must be the last line in qrencode_functions[] */
};
/* }}} */

/* {{{ qrencode_module_entry
 */
zend_module_entry qrencode_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_QRENCODE_EXTNAME,
    qrencode_functions,
    PHP_MINIT(qrencode),
    PHP_MSHUTDOWN(qrencode),
    NULL,
    NULL,
    PHP_MINFO(qrencode),
    PHP_QRENCODE_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_QRENCODE
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(qrencode)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
