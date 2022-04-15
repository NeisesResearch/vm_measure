

add_library(sph STATIC
    blake.c bmw.c cubehash.c echo.c fugue.c groestl.c hamsi.c haval.c jh.c keccak.c luffa.c md2.c md4.c md5.c panama.c radiogatun.c ripemd.c sha0.c sha1.c sha2.c sha2big.c shabal.c shavite.c simd.c skein.c tiger.c whirlpool.c)
target_link_libraries(sph)
