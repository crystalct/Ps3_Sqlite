/*
 *  Copyright (C) 2007-2015 Lonelycoder AB
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  This program is also available under a commercial proprietary license.
 *  For more information, contact andreas@lonelycoder.com
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "buf.h"
#include "str.h"
#include "main.h"
#include "sha.h"
//#include "i18n.h"

//#include "unicode_casefolding.h"
//#include "charset_detector.h"
//#include "big5.h"
#include "arch.h"

//#include <libavformat/avformat.h> // for av_url_split()

const static charset_t charsets[];

char *
find_str(const char *s, int len, const char *needle)
{
  int nlen = strlen(needle);
  if(len < nlen)
    return NULL;

  len -= nlen;
  for(int i = 0; i <= len; i++) {
    int j;
    for(j = 0; j < nlen; j++) {
      if(s[i+j] != needle[j]) {
        break;
      }
    }
    if(j == nlen)
      return (char *)s + i;
  }
  return NULL;
}


