import { useLayoutEffect, useState } from "react";

import { createTheme, ThemeProvider } from "@mui/material/styles";
import AppBar from '@mui/material/AppBar';
import Box from '@mui/material/Box';
import Toolbar from '@mui/material/Toolbar';
import IconButton from '@mui/material/IconButton';
import Typography from '@mui/material/Typography';
import Menu from '@mui/material/Menu';
import MenuIcon from '@mui/icons-material/Menu';
import Container from '@mui/material/Container';
import Button from '@mui/material/Button';
import MenuItem from '@mui/material/MenuItem';

import { useNavigate } from "react-router-dom";
import { LogoElement, UserAccount } from './topBar';

// ...

// constants
const kPages = [ /* ... */ ];

// ...

/**
 * Main function
 */
export default function HomePage({ showLogInDialog = false, showSignUpDialog = false }) {
  // hooks
  // ...

  const [anchorElNav, setAnchorElNav] = useState<null | HTMLElement>(null);

  // function used to navigate to other pages
  let navigate = useNavigate();

  // navigation menu state (open/closed)
  const handleOpenNavMenu = (event: React.MouseEvent<HTMLElement>) => {
    setAnchorElNav(event.currentTarget);
  }

  const handleCloseNavMenu = () => {
    setAnchorElNav(null);
  }

  useLayoutEffect(() => {
    // auto login
    // ...
  }, [])

  function LogInButtons() {
    // ...
  }

  return (
    <ThemeProvider theme={kTheme}>
      <AppBar position="static" color="primary">
        <Container maxWidth="xl">
          <Toolbar disableGutters>

            {/* Logo */}
            {LogoElement()}

            {/* Menu */}
            <Box sx={{ flexGrow: 1, display: { xs: 'flex', md: 'none' } }}>
              <IconButton size="large" aria-label="account of current user" aria-controls="menu-appbar" aria-haspopup="true" onClick={handleOpenNavMenu} color="inherit">
                <MenuIcon />
              </IconButton>
              <Menu id="menu-appbar" anchorEl={anchorElNav} anchorOrigin={{ vertical: 'bottom', horizontal: 'left', }} 
                    keepMounted transformOrigin={{ vertical: 'top', horizontal: 'left', }} open={Boolean(anchorElNav)}
                    onClose={handleCloseNavMenu} sx={{ display: { xs: 'block', md: 'none' }, }}
              >
                {kPages.map((page) => (
                  <MenuItem key={page} onClick={handleCloseNavMenu}>
                    <Typography textAlign="center">{page}</Typography>
                  </MenuItem>
                ))}
              </Menu>
            </Box>

            {/* Navigation buttons */}
            {LogoElement(true)}
            <Box sx={{ flexGrow: 1, display: { xs: 'none', md: 'flex' } }}>
              {kPages.map((page) => (
                <Button key={page} onClick={handleCloseNavMenu} sx={{ my: 1, color: '#a4ccf4', display: 'block', textTransform: 'none', ':hover': { color: 'white', } }}>
                  {page}
                </Button>
              ))}
            </Box>

            {LogInButtons()}
            {UserAccount(showProfile, handleLogout)}
          </Toolbar>
        </Container>
      </AppBar>
    </ThemeProvider>
  );
}