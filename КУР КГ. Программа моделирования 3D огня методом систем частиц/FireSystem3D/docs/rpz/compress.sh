#!/bin/bash

# Проверка наличия Ghostscript
if ! command -v gs &> /dev/null; then
    echo "Ошибка: Ghostscript не установлен" >&2
    exit 1
fi

# Поиск PDF файлов в текущей директории
find . -maxdepth 1 -name "*.pdf" -type f | while read -r file; do
    if [ -f "$file" ] && [ -r "$file" ]; then
        echo "Сжатие: $file"
        
        # Сжатие с помощью Ghostscript
        gs -sDEVICE=pdfwrite \
           -dCompatibilityLevel=1.4 \
           -dNOPAUSE \
           -dOptimize=true \
           -dQUIET \
           -dBATCH \
           -dRemoveUnusedFonts=true \
           -dRemoveUnusedImages=true \
           -dOptimizeResources=true \
           -dDetectDuplicateImages \
           -dCompressFonts=true \
           -dEmbedAllFonts=true \
           -dSubsetFonts=true \
           -dPreserveAnnots=true \
           -dPreserveMarkedContent=true \
           -dPreserveOverprintSettings=true \
           -dPreserveHalftoneInfo=true \
           -dPreserveOPIComments=true \
           -dPreserveDeviceN=true \
           -dMaxInlineImageSize=0 \
           -sOutputFile=./tmp.tmp \
           "$file"
        
        # Замена оригинального файла
        if [ $? -eq 0 ] && [ -f "./tmp.tmp" ]; then
            mv "./tmp.tmp" "$file"
            echo "Успешно: $file"
        else
            echo "Ошибка при сжатии: $file" >&2
            [ -f "./tmp.tmp" ] && rm -f "./tmp.tmp"
        fi
    fi
done
