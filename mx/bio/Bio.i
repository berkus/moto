Extension: Bio 

Include: "motoBio.h"
Library: -lssl

Interface:

/** Creates a new default NULL Bio source/sink. **/
tracked Bio Bio::Bio() =>
        Bio* motoBio_createDefault();

/** Creates a new Bio of the specified type <i>type</i>. See
<b>bio.defines</b> for type definitions. **/
tracked Bio Bio::Bio(int type) =>
        Bio* motoBio_createNew(int type);

void Bio::~Bio() => 
        void motoBio_free(Bio *this);

/** Creates a Bio chain of the order: Bio -> pushed Bio **/
void Bio::push(Bio bio) =>
        void motoBio_push(Bio *this, Bio *bio);

/** Removes the given Bio <i>bio</i> from the Bio chain. The Bio to remove can
be anywhere in the chain **/
void Bio::pop(Bio bio) =>
        void motoBio_pop(Bio *this, Bio *bio);

/** Removes the last Bio in the chain and returns a reference to it **/
tracked Bio Bio::pop() =>
        void motoBio_popLast(Bio *this);

/** Removes the Bio at location <i>index</i> in the chain and returns a
reference to it. **/
tracked Bio Bio::popBioAt(int index) =>
        Bio* motoBio_popBioAt(Bio *this, int index);

/** Returns a reference to a given Bio at location <i>index</i> in the Bio the
chain. This does not remove it from the chain. **/
tracked Bio Bio::getBioAt(int index) =>
        Bio* motoBio_getBioAt(Bio *this, int index);

/** Sets the cipher to use for a CIPHER Bio. Cipher types can be found in
<b>bio.defines</b>. **/
void Bio::setCipher(int type) =>
        void motoBio_setCipher(Bio *this, int type);

/** Sets the digest to use for a DIGEST Bio. Digest types can be found in
<b>bio.defines</b>. **/
void Bio::setDigest(int type) =>
        void motoBio_setDigest(Bio *this, int type);

/** Sets the file and mode to use for a FSTREAM Bio. **/
void Bio::setFile(String file, int mode) =>
        void motoBio_setFile(Bio *this, char *file, int mode);

/** Sets the size of the internal buffer to read to. The default size is 1024.
This is the smallest the buffer can be. **/
void Bio::setBufferSize(int size) =>
        void motoBio_setBufferSize(Bio *this, int size);

/** Returns the size of the internal buffer. **/
int Bio::getBufferSize() =>
        int motoBio_getBufferSize(Bio *this);

/** Sets the encryption key to use for a given CIPHER Bio. **/
void Bio::setKey(String key) =>
        void motoBio_setKey(Bio *this, const unsigned char *key);

/** Gets the encryption key from a given CIPHER Bio. **/
String Bio::getKey() =>
        unsigned char* motoBio_getKey(Bio *this);

/** Writes out <i>data</i> to a given Bio chain. **/
int Bio::write(String data) =>
        int motoBio_write(Bio *this, const char *data);

/** Writes out <i>data</i> to a given CIPHER Bio chain. **/
int Bio::write(String data, String key) =>
        int motoBio_writeEnc(Bio *this, const char *data, const unsigned char *key);

/** Reads from a given Bio chain. **/
tracked byte[] Bio::read() =>
        UnArray* motoBio_read(Bio *this);

/** Returns the number of bytes returned from a previous <i>read()</i>. **/
int Bio::getBytesRead() =>
        int motoBio_getBytesRead(Bio *this);

/** Flushes any buffered data left from a previous <i>write()</i> call. This
is required on most Bios. **/
void Bio::flush() =>
        void motoBio_flush(Bio *this);

/** Resets the internal state of a given Bio. **/
void Bio::reset() =>
        void motoBio_reset(Bio *this);

/** Returns <b>true</b> if the Bio has reached <i>eof</i>, <b>false</b>
otherwise. Eof can have different meanings depending on the Bio type. **/
boolean Bio::eof() =>
        int motoBio_eof(Bio *this);

/** Returns the size of a given Bio chain. **/
int Bio::size() =>
        int motoBio_size(Bio *this);

/** Creates a Bio chain of the types specified in <i>types</i> array. **/
void Bio::createChain(int[] types) =>
        void motoBio_createChain(Bio *this, UnArray *types);


