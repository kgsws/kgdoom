LIBTRANSISTOR_HOME=../../dist/
PROGRAM := kgdoom
OBJ := am_map.o hu_stuff.o m_menu.o p_maputl.o p_user.o st_lib.o i_main.o m_misc.o p_mobj.o r_bsp.o st_stuff.o d_main.o m_random.o r_data.o tables.o d_net.o info.o m_swap.o p_pspr.o r_draw.o v_video.o i_sound.o r_main.o wi_stuff.o doomstat.o i_system.o p_setup.o r_plane.o w_wad.o dstrings.o i_video.o p_enemy.o p_sight.o r_segs.o z_zone.o f_finale.o m_argv.o p_spec.o r_sky.o m_bbox.o p_inter.o p_switch.o r_things.o g_game.o m_fixed.o m_cheat.o p_map.o p_tick.o s_sound.o p_pickup.o p_generic.o network.o cl_cmds.o p_inventory.o kg_lua.o kg_3dfloor.o kg_record.o kg_text.o t_text.o t_ini.o

include $(LIBTRANSISTOR_HOME)/libtransistor.mk

build: liblua.a $(PROGRAM).nro kgdoom.wad

include lua5.3.4/liblua.mk

%.o: %.c
	$(CC) $(CC_FLAGS) -Wno-pointer-arith -c -o $@ $<

%.o: %.S
	$(AS) $(AS_FLAGS) $< -filetype=obj -o $@

$(PROGRAM).nro.so: ${OBJ} $(LIBTRANSITOR_NRO_LIB) $(LIBTRANSISTOR_COMMON_LIBS) liblua.a
	$(LD) $(LD_FLAGS) -o $@ ${OBJ} $(LIBTRANSISTOR_NRO_LDFLAGS) liblua.a -lm

kgdoom.wad:
	wadsrc/wadpack.py wadsrc/kgdoom

clean:
	rm -rf *.o *.nso *.nro *.so lua5.3.4/*.o *.a *.d kgdoom.wad
