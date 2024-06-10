import 'dart:ffi';
import 'dart:io';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';

enum LC3DecoderError {
  lc3DecoderOk(0),
  lC3DecoderNotInitialized(1),
  lC3DecoderInvalidParams(2),
  lC3DecoderMemoryError(3),
  lc3DecoderInternalError(4);

  final int value;

  const LC3DecoderError(this.value);

  static LC3DecoderError fromValue(int value) {
    return values.firstWhere(
      (element) => element.value == value,
      orElse: () => throw ArgumentError("Invalid value: $value"),
    );
  }
}

typedef DecoderInitC = Int32 Function(Uint8, Uint8, Uint32, Uint16, Uint32);
typedef DecoderInitDart = int Function(int, int, int, int, int);

typedef DecoderDecodeC = Int32 Function(Pointer<Uint8>, Pointer<Uint8>);
typedef DecoderDecodeDart = int Function(Pointer<Uint8>, Pointer<Uint8>);

typedef DecoderDestroyC = Void Function();
typedef DecoderDestroyDart = void Function();

typedef DecoderGetBlockBytesC = Int32 Function();
typedef DecoderGetBlockBytesDart = int Function();

typedef DecoderGetFrameSamplesC = Int32 Function();
typedef DecoderGetFrameSamplesDart = int Function();

class LC3DecoderBinding {
  bool _isInitialized = false;
  late final DynamicLibrary _lc3Internal;
  late final DecoderInitDart _decoderInit;
  late final DecoderDecodeDart _decoderDecode;
  late final DecoderDestroyDart _decoderDestroy;
  late final DecoderGetBlockBytesDart _decoderGetBlockBytes;
  late final DecoderGetFrameSamplesDart _decoderGetFrameSamples;

  Pointer<Uint8> _encodedData = nullptr;
  int _encodedDataSize = 0;
  Pointer<Uint8> _decodedData = nullptr;
  int _decodedDataSize = 0;

  LC3DecoderBinding._private();

  static final LC3DecoderBinding _instance = LC3DecoderBinding._private();

  static LC3DecoderBinding get instance => _instance;

  void checkInitialized() {
    if (!_isInitialized) {
      throw Exception("LC3DecoderBinding is not initialized");
    }
  }

  void initialize() {
    if (_isInitialized) {
      return;
    }

    const String libName = 'liblc3_dart_plugin';

    /// The dynamic library in which the symbols for [Liblc3DartPluginBindings] can be found.
    _lc3Internal = () {
      if (Platform.isMacOS || Platform.isIOS) {
        return DynamicLibrary.open('$libName.framework/$libName');
      }
      if (Platform.isAndroid || Platform.isLinux) {
        return DynamicLibrary.open('lib$libName.so');
      }
      if (Platform.isWindows) {
        return DynamicLibrary.open('$libName.dll');
      }
      throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
    }();

    _decoderInit = _lc3Internal
        .lookupFunction<DecoderInitC, DecoderInitDart>("decoder_init");
    _decoderDecode = _lc3Internal
        .lookupFunction<DecoderDecodeC, DecoderDecodeDart>("decoder_decode");
    _decoderDestroy = _lc3Internal
        .lookupFunction<DecoderDestroyC, DecoderDestroyDart>("decoder_destroy");
    _decoderGetBlockBytes = _lc3Internal.lookupFunction<DecoderGetBlockBytesC,
        DecoderGetBlockBytesDart>("decoder_get_block_bytes");
    _decoderGetFrameSamples = _lc3Internal.lookupFunction<
        DecoderGetFrameSamplesC,
        DecoderGetFrameSamplesDart>("decoder_get_frame_samples");
    _isInitialized = true;
  }

  int get encodedDataSize => _encodedDataSize;
  int get decodedDataSize => _decodedDataSize;

  void decoderInit(
      int sampleRate, int channels, int frameSize, int bitDepth, int bitRate) {
    checkInitialized();
    _assertDecoderResult(_decoderInit(
      bitDepth,
      channels,
      sampleRate,
      frameSize,
      bitRate,
    ));
    if (_encodedData != nullptr) {
      calloc.free(_encodedData);
    }
    if (_decodedData != nullptr) {
      calloc.free(_decodedData);
    }
    _encodedDataSize = _decoderGetBlockBytes();
    _encodedData = calloc<Uint8>(_encodedDataSize);
    _decodedDataSize = _decoderGetFrameSamples() * channels * bitDepth ~/ 8;
    _decodedData = calloc<Uint8>(_decodedDataSize);
  }

  Uint8List decoderDecode(Iterable<int> encodedData) {
    checkInitialized();
    _encodedData.asTypedList(_encodedDataSize).setAll(0, encodedData);
    _assertDecoderResult(_decoderDecode(_encodedData, _decodedData));
    final result = Uint8List(_decodedDataSize);
    result.setAll(0, _decodedData.asTypedList(_decodedDataSize));
    return result;
  }

  void decoderDestroy() {
    checkInitialized();
    _decoderDestroy();
    if (_encodedData != nullptr) {
      calloc.free(_encodedData);
      _encodedData = nullptr;
      _encodedDataSize = 0;
    }
    if (_decodedData != nullptr) {
      calloc.free(_decodedData);
      _decodedData = nullptr;
      _decodedDataSize = 0;
    }
  }

  void _assertDecoderResult(int result) {
    if (result != 0) {
      throw LC3DecoderBindingError(LC3DecoderError.fromValue(result));
    }
  }
}

abstract class LC3DecoderBindingException implements Exception {
  final String message;

  LC3DecoderBindingException(this.message);

  @override
  String toString() {
    return "LC3DecoderBindingException: $message";
  }
}

class LC3DecoderBindingNotInitializedException
    extends LC3DecoderBindingException {
  LC3DecoderBindingNotInitializedException()
      : super("LC3DecoderBinding is not initialized");
}

class LC3DecoderBindingDynamicLibraryOpenException
    extends LC3DecoderBindingException {
  LC3DecoderBindingDynamicLibraryOpenException(String path)
      : super("Failed to open dynamic library at path: $path");
}

class LC3DecoderBindingFunctionLookupException
    extends LC3DecoderBindingException {
  LC3DecoderBindingFunctionLookupException(String functionName)
      : super("Failed to lookup function: $functionName");
}

class LC3DecoderBindingError extends LC3DecoderBindingException {
  LC3DecoderBindingError(LC3DecoderError error) : super("Error: $error");
}
