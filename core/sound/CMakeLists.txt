set (SOURCE sound.c sound-libvlc.c)

add_library (sound STATIC ${SOURCE})

target_link_libraries(
	sound
    vlc
)
