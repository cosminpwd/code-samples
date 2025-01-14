# Single page application using React and Material UI
The main function of the app which contains the app bar with the logo, navigation items and user account options. Main page content is not included in the sample.

## Details
- The `LogoElement()` and `UserAccount()` custom components are implemented in another `.tsx` file.
- The `LogInButtons()` component is implemented locally (included in the sample).
- Using React Router for navigation, the app navigates to another page only on a successful login.
- On `useLayoutEffect()` the app attempts an automatic log in (not included in the sample).