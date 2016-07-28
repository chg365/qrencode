#!/usr/local/chg/base/opt/php/bin/php
<?php
$url = 'https://github.com/chg365/qrencode/archive/0.1.0.tar.gz';
$qr = qr_encode ($url);
if (is_resource ($qr))
{
    $flag = qr_save ($qr, '1.png');
    //header ("Content-type: image/PNG");
    //$flag = qr_save ($qr);
    var_dump($flag);
    exit;
}
exit;
/*
resource = qr_encode (string $text, [ int $version, int $mode, int $casesensitive]);

bool = qr_save (resource $qr, [ string $filename] );
*/
