qrencode
==============

A php qrencode extension


这个程序是在https://github.com/dreamsxin/qrencodeforphp的基础上修改而来。
原因是在php7上编译不过去。

在php7.0.7、php5.4.20上测试能编译过去

Version：
最大值为40，最小值为1

Mode:
QR_MODE_NUL
QR_MODE_NUM
QR_MODE_AN
QR_MODE_8
QR_MODE_KANJI

Level:
QR_ECLEVEL_L
QR_ECLEVEL_M
QR_ECLEVEL_Q
QR_ECLEVEL_H

示例：

```php
$qr = qr_encode('test for qrcode');
if (is_resource ($qr))
    qr_save ($qr, '1.png');
```

```php
$qr = qr_encode ('test for qrcode');
if (is_resource ($qr))
{
    header ("Content-type: image/PNG");
    qr_save ($qr);
}
```

```php
<?php
$qr = qr_encode ('test for qrcode', 1, QR_ECLEVEL_M, QR_MODE_KANJI, 1);
if (is_resource ($qr))
{
    header ("Content-type: image/PNG");
    qr_save ($qr);
}
```

qr_encode (text, version, level, mode, casesensitive);
/**
* @param text the string want to encode.
* @param version qrcode version, default 1
* @param level level default, QR_ECLEVEL_L
* @param mode mode, could be QR_MODE_NUM, QR_MODE_AN, QR_MODE_8, QR_MODE_KANJI. default QR_MODE_8
* @param casesensitive casesentive, if you want the 8 bit mode, must set this on. default 1
* @return 
*/
