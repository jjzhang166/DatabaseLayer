
MAKE := make --no-print-directory

all :
	@$(MAKE) -f makefile_mysql_share_debug clean;
	@$(MAKE) -f makefile_mysql_share_debug;
	@$(MAKE) -f makefile_mysql_share_release clean;
	@$(MAKE) -f makefile_mysql_share_release;
	@$(MAKE) -f makefile_mysql_static_debug clean;
	@$(MAKE) -f makefile_mysql_static_debug;
	@$(MAKE) -f makefile_mysql_static_release clean;
	@$(MAKE) -f makefile_mysql_static_release;
	@$(MAKE) -f makefile_sqllite3_share_debug clean;
	@$(MAKE) -f makefile_sqllite3_share_debug;
	@$(MAKE) -f makefile_sqllite3_share_release clean;
	@$(MAKE) -f makefile_sqllite3_share_release;
	@$(MAKE) -f makefile_sqllite3_static_release clean;
	@$(MAKE) -f makefile_sqllite3_static_release;
	@$(MAKE) -f makefile_sqllite3_static_debug clean;
	@$(MAKE) -f makefile_sqllite3_static_debug;

clean:
	@$(MAKE) -f makefile_mysql_share_debug clean;
	@$(MAKE) -f makefile_mysql_share_release clean;
	@$(MAKE) -f makefile_mysql_static_debug clean;
	@$(MAKE) -f makefile_mysql_static_release clean;
	@$(MAKE) -f makefile_sqllite3_share_debug clean;
	@$(MAKE) -f makefile_sqllite3_share_release clean;
	@$(MAKE) -f makefile_sqllite3_static_release clean;
	@$(MAKE) -f makefile_sqllite3_static_debug clean;
