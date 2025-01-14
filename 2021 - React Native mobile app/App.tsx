import React, { useState, useEffect, useRef } from 'react';
import {
  Text, Switch, ScrollView, Button, TextInput,
  StyleSheet, View, Pressable, LogBox, ActivityIndicator,
  Keyboard, FlatList, PermissionsAndroid, Image
} from 'react-native';

import FontAwesome5Icon from 'react-native-vector-icons/FontAwesome5';

import CalendarPicker from 'react-native-calendar-picker';
import Geolocation from 'react-native-geolocation-service';
import Icon from 'react-native-vector-icons/FontAwesome5';
import moment from 'moment';

import { findClosestAirport, getAirportsStr } from './Aiports';
import { AutocompleteInput } from './InputAutoComplete';

// constants
// ...

const RenderMainView = () => {
  // hooks
  const mMainScrollViewRef = useRef(null);
  const mSearchButtonRef = useRef(null);

  const [mFromAirport, setFromAirport] = useState('');
  const [mToAirport, setToAirport] = useState('');
  const [mSuggestedFromAirports, updateFromAirports] = useState([]);
  const [mSuggestedToAirports, updateToAirports] = useState([]);

  const [mShowCalendar, setShowCalendar] = useState(false);
  const [mSelectedDate, setSelectedDate] = useState(moment().format('YYYY-MM-DD'));

  const [mIsOneWay, setIsOneWay] = useState(false);
  const [mShowReturnDate, setShowReturnDate] = useState(true);
  const [mShowReturnCalendar, setShowReturnCalendar] = useState(false);
  const [mSelectedReturnDate, setSelectedReturnDate] = useState<string>("");

  const [mPssgCount, setPssgCount] = useState('1');
  const [mIsFirstClass, setIsFirstClass] = useState(false);
  const [mIsEarlyFlight, setIsEarlyFlight] = useState(false);

  const [mIsSearchingFlights, setIsSearchingFlights] = useState(false);
  const [mIsSearchPerformed, setIsSearchPerformed] = useState(false);
  const [mIsFlightsResultVisible, setIsFlightsResultsVisible] = useState(false);
  const [mSearchScrollPos, setSearchScrollPos] = useState(null);

  const [mIsEmptyFromAirport, setIsEmptyFromAirport] = useState(false);
  const [mIsEmptyToAirport, setIsEmptyToAirport] = useState(false);

  // main containers:
  // - flight result: map[id, offer]
  // - flight result simple: [price/time]
  // - flights per price: map[price, [id]]
  // - flight stops: map[id, [stopIndex]]
  const [mFlightsResult, setFlightsResult] = useState(new Map());
  const [mFlightsResultSortedIds, setFlightsResultSortedIds] = useState([]);
  const [mFlightsResultVisible, setFlightsResultVisible] = useState([]);
  const [mFlightsPerPrice, setFlightsPerPrice] = useState(new Map());
  const [mFlightStops, setFlightStops] = useState(new Map());

  // ---------- INIT FUNCTIONS ---------------------------------------------------------------------

  const getCurrentLocation = () => {
    Geolocation.getCurrentPosition((position) => {
      // determine closest airport
      if (!mFromAirport)
        setFromAirport(findClosestAirport(position.coords.latitude, position.coords.longitude));
    },
      (error) => {
        console.log(error.code, error.message);
      },
      { enableHighAccuracy: true, timeout: 15000, maximumAge: 10000 }
    );
  }

  const requestLocationPermissions = async () => {
    try {
      const granted = await PermissionsAndroid.request(
        PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
        {
          title: 'Current Location Permission',
          message:
            'App needs access to your current location ' +
            'to find the nearest airport.',
          buttonNeutral: 'Ask Me Later',
          buttonNegative: 'Cancel',
          buttonPositive: 'OK'
        }
      );
      if (granted === PermissionsAndroid.RESULTS.GRANTED) {
        console.log('Fine location permission granted');
        getCurrentLocation();
      } else {
        console.log('Fine location permission denied.');
        console.log(granted);
      }
    } catch (err) {
      console.warn(err);
    }
  };

  // ---------- EFFECTS ----------------------------------------------------------------------------

  useEffect(() => {
    requestLocationPermissions();
  }, [])

  // ---------- COMPONENTS -------------------------------------------------------------------------

  const calendarView = (dateType: string) => {
    // base calendar for flight date
    if (dateType === 'base') {
      if (!mShowCalendar) { return null; }

      return (<CalendarPicker
        // calendar options, see documentation
        startFromMonday={true}
        scrollable={true}
        selectedDayColor={'#00b1d2ff'}
        selectedDayTextColor={'white'}
        todayBackgroundColor={'white'}
        todayTextStyle={mSelectedDate === moment().format('YYYY-MM-DD') ? { color: 'white' } : { color: 'black' }}
        previousTitle={'ᐸ'}
        nextTitle={'ᐳ'}
        previousTitleStyle={{ color: '#00b1d2ff' }}
        nextTitleStyle={{ color: '#00b1d2ff' }}
        minDate={moment().format('YYYY-MM-DD')}

        // Initially visible month. Default = Date(). Using https://momentjs.com for formatting.
        selectedStartDate={mSelectedDate}

        // Handler which gets executed on day press. Default = undefined
        onDateChange={(day) => {
          setSelectedDate(moment(day).format('YYYY-MM-DD'));
          setShowCalendar(false);
        }}
      />);
    }

    // calendar for return date
    if (!mShowReturnCalendar) { return null; }

    return (<CalendarPicker
      // calendar options, see documentation
      startFromMonday={true}
      scrollable={true}
      selectedDayColor={'#00b1d2ff'}
      selectedDayTextColor={'white'}
      todayBackgroundColor={'white'}
      todayTextStyle={mSelectedReturnDate === moment().format('YYYY-MM-DD') ? { color: 'white' } : { color: 'black' }}
      previousTitle={'ᐸ'}
      nextTitle={'ᐳ'}
      previousTitleStyle={{ color: '#00b1d2ff' }}
      nextTitleStyle={{ color: '#00b1d2ff' }}
      minDate={moment().format('YYYY-MM-DD')}

      // Initially visible month. Default = Date(). Using https://momentjs.com for formatting.
      selectedStartDate={mSelectedReturnDate}

      // Handler which gets executed on day press. Default = undefined
      onDateChange={(day) => {
        setSelectedReturnDate(moment(day).format('YYYY-MM-DD'));
        setShowReturnCalendar(false);
      }}
    />);
  }

  // return date
  const returnDateView = () => {
    if (!mShowReturnDate) {
      return null;
    }

    return (
      <ScrollView>
        <View style={styles.inputView}>
          <Icon style={styles.inputIcon} name="calendar-plus" size={20} />
          <TextInput
            style={styles.input}
            placeholder="Return date"
            value={mSelectedReturnDate ? moment(mSelectedReturnDate).format('DD MMMM YYYY') : ''}
            caretHidden={true}
            showSoftInputOnFocus={false}
            contextMenuHidden={true}
            onPressIn={() => {
              if (mShowReturnCalendar) { setShowReturnCalendar(false); }
              else {
                if (mShowCalendar) { setShowCalendar(false); }
                setShowReturnCalendar(true);
              }
            }}
          />
        </View>

        {/* Calendar */}
        {calendarView('return')}
      </ScrollView>);
  }

  // one-way toggle
  const toggleOneWaySwitch = () => {
    if (mIsOneWay) {
      setIsOneWay(false);
      setShowReturnDate(true);
    }
    else {
      setIsOneWay(true);
      if (mShowReturnCalendar) { setShowReturnCalendar(false); }

      setSelectedReturnDate('');
      setShowReturnDate(false);
    }
  }

  // economy/first-class toggle
  const toggleFirstClassSwitch = () => {
    if (mIsFirstClass) { setIsFirstClass(false); }
    else { setIsFirstClass(true); }
  }

  // bargain/early toggle
  const toggleEarlyFlightSwitch = () => {
    if (mIsEarlyFlight) { setIsEarlyFlight(false); }
    else { setIsEarlyFlight(true); }
  }

  // search loading
  const searchProgress = () => {
    if (mIsSearchingFlights) { return (<ActivityIndicator style={styles.searchButtonLoading} size="large" animating={true} color='deepskyblue' />); }

    return null;
  }

  // flight duration
  const getFlightDuration = (departingAt: moment.MomentInput, arrivingAt: moment.MomentInput) => {
    let hours = Math.floor(moment.duration(moment(arrivingAt).diff(moment(departingAt))).asHours());
    let minutes = moment.duration(moment(arrivingAt).diff(moment(departingAt))).asMinutes() - hours * 60;
    let hoursStr = (hours > 9) ? hours : ('0' + hours);
    let minutesStr = (minutes > 9) ? minutes : ('0' + minutes);

    return (hoursStr + 'h' + minutesStr + 'min');
  }

  const loadMoreSearchResults = () => {
    if (mFlightsResultVisible.length == mFlightsResultSortedIds.length) { return; }

    setFlightsResultVisible(mFlightsResultSortedIds.slice(0, mFlightsResultVisible.length + 5));
  }

  // list of stops for a flight
  const stopsList = (flightId: string) => {
    return (
      <FlatList
        keyboardShouldPersistTaps='always'
        data={mFlightStops.get(flightId)}
        renderItem={({ item, index }) => (
          <Text style={styles.flightTimesTextSmall}>
            {mFlightsResult.get(flightId)['slices'][0]['segments'][item]['origin']['iata_city_code']} - {mFlightsResult.get(flightId)['slices'][0]['segments'][item]['destination']['iata_city_code']}
          </Text>
        )}
      />
    );
  }

  // list of plane IDs for a flight
  const aircraftList = (flightId: string) => {
    return (
      <FlatList
        keyboardShouldPersistTaps='always'
        data={mFlightStops.get(flightId)}
        renderItem={({ item, index }) => (
          <Text style={styles.flightTimesTextSmall}>
            {mFlightsResult.get(flightId)['slices'][0]['segments'][item]['marketing_carrier']['iata_code']}{mFlightsResult.get(flightId)['slices'][0]['segments'][item]['marketing_carrier_flight_number']}
          </Text>
        )}
      />
    );
  }

  // flights found
  const flightsResult = () => {
    if (mIsEmptyFromAirport) { return (<Text style={styles.validationErrMessage}>{'Please select a departure airport.'}</Text>); }

    if (mIsEmptyToAirport) { return (<Text style={styles.validationErrMessage}>{'Please select a destination airport.'}</Text>); }

    if (!mIsFlightsResultVisible) {
      if (mIsSearchPerformed) {
        if (mMainScrollViewRef.current) mMainScrollViewRef.current.scrollToEnd({ animated: true });
        return (<Text style={styles.validationErrMessage}>No flights found for {moment(mSelectedDate).format('DD MMMM YYYY')}</Text>);
      }

      return null;
    }

    return (
      <FlatList
        style={styles.flightResultsList}
        keyboardShouldPersistTaps='always'
        data={mFlightsResultVisible}
        onEndReached={loadMoreSearchResults}
        renderItem={({ item, index }) => (
          <Pressable
            style={styles.flightResultCard}
            onPress={() => { }}>
            <View style={styles.flightsAvailableTimeView}>
              <View style={styles.flightsAvailableTimeView}>
                <Text style={styles.flightTimesText}>
                  {moment(mFlightsResult.get(item)['slices'][0]['segments'][0]['departing_at']).format('kk:mm')} - {moment(mFlightsResult.get(item)['slices'][0]['segments'][mFlightsResult.get(item)['slices'][0]['segments'].length - 1]['arriving_at']).format('kk:mm')}
                </Text>
                <Text style={styles.flightTimesTextDetails}>   {(mFlightsResult.get(item)['slices'][0]['segments'].length > 1) ? (mFlightsResult.get(item)['slices'][0]['segments'].length + ' stops') : 'Non-stop'}</Text>
              </View>
              <Text style={[styles.flightTimesText, { textAlign: 'right', textAlignVertical: 'bottom' }]}>
                {getFlightDuration(mFlightsResult.get(item)['slices'][0]['segments'][0]['departing_at'], mFlightsResult.get(item)['slices'][0]['segments'][mFlightsResult.get(item)['slices'][0]['segments'].length - 1]['arriving_at'])}
              </Text>
            </View>
            <View style={styles.flightsAvailableTimeView}>
              <View style={styles.flightsAvailableTimeView}>
                {stopsList(item)}
              </View>
              <View style={styles.flightsAvailableTimeView}>
                {aircraftList(item)}
              </View>
            </View>
            <Text style={styles.flightPrice}>{mFlightsResult.get(item)['total_amount'] + mFlightsResult.get(item)['total_currency']}</Text>
          </Pressable>
        )}
      />
    );
  }

  // ---------- MAIN VIEW --------------------------------------------------------------------------
  return (
    <ScrollView
      style={styles.scrollView}
      keyboardShouldPersistTaps='always'
      ref={mMainScrollViewRef}>

      {/* Logo */}
      <View style={styles.logoView}>
        <Image
          source={require('./img/logo.png')}
          style={styles.logo}
        />
      </View>

      {/* SubTitle */}
      <Text style={styles.appSubTitle}>Book flights on the go</Text>

      {/* From */}
      <AutocompleteInput
        iconName="plane-departure"
        autoCapitalize="none"
        autoCorrect={false}
        keyboardShouldPersistTaps='always'
        iconStyle={styles.inputIcon}
        containerStyle={styles.inputView}
        listContainerStyle={styles.autocompleteInputList}
        inputContainerStyle={styles.autocompleteInput}
        data={mSuggestedFromAirports}
        defaultValue={mFromAirport}
        value={mFromAirport}
        onChangeText={(text: React.SetStateAction<string>) => {
          setFromAirport(text);
          updateFromAirports(getAirportsStr(text));
          setIsEmptyFromAirport(mFromAirport.length == 0);
        }}
        placeholder="From"
        flatListProps={{
          renderItem: ({ item }) => (
            <Pressable
              onPress={() => {
                setFromAirport(item);
                updateFromAirports([]);
              }}>
              <Text style={styles.suggestedText}>{item}</Text>
            </Pressable>
          ),
        }}
      />

      {/* To */}
      <AutocompleteInput
        iconName="plane-arrival"
        autoCapitalize="none"
        autoCorrect={false}
        keyboardShouldPersistTaps='always'
        iconStyle={styles.inputIcon}
        containerStyle={styles.inputView}
        listContainerStyle={styles.autocompleteInputList}
        inputContainerStyle={styles.autocompleteInput}
        data={mSuggestedToAirports}
        defaultValue={mToAirport}
        value={mToAirport}
        onChangeText={(text: React.SetStateAction<string>) => {
          setToAirport(text);
          updateToAirports(getAirportsStr(text));
          setIsEmptyToAirport(mToAirport.length == 0);
        }}
        placeholder="To"
        flatListProps={{
          renderItem: ({ item }) => (
            <Pressable
              onPress={() => {
                setToAirport(item);
                updateToAirports([]);
              }}>
              <Text style={styles.suggestedText}>{item}</Text>
            </Pressable>
          ),
        }}
      />

      {/* Date */}
      <View style={styles.inputView}>
        <Icon style={styles.inputIcon} name="calendar" size={20} />
        <TextInput
          style={styles.input}
          placeholder="Date"
          defaultValue={moment(mSelectedDate).format('DD MMMM YYYY')}
          caretHidden={true}
          showSoftInputOnFocus={false}
          contextMenuHidden={true}
          onPressIn={() => {
            if (mShowCalendar) { setShowCalendar(false); }
            else {
              if (mShowReturnCalendar) { setShowReturnCalendar(false); }
              setShowCalendar(true);
            }
          }}
        />
      </View>

      {/* Calendar */}
      {calendarView('base')}

      {/* One-way switch */}
      <View style={styles.switch}>
        <Text style={mIsOneWay ? styles.textInactive : styles.textActive}>Return</Text>
        <Switch
          trackColor={{ false: '#cccccc', true: 'skyblue' }}
          thumbColor={mIsOneWay ? '#ffd616' : '#837970'}
          ios_backgroundColor="#3e3e3e"
          onValueChange={toggleOneWaySwitch}
          value={mIsOneWay}
          style={{ marginLeft: 10, marginRight: 10 }}
        />
        <Text style={mIsOneWay ? styles.textActive : styles.textInactive}>One-way</Text>
      </View>

      {/* Return Date */}
      {returnDateView()}

      {/* Passengers */}
      <View style={styles.inputView}>
        <Icon style={styles.inputIcon} name="user-friends" size={20} />
        <TextInput
          style={styles.input}
          onChangeText={setPssgCount}
          value={parseInt(mPssgCount, 10) + (parseInt(mPssgCount, 10) > 1 ? ' passengers' : ' passenger')}
          placeholder="Passengers"
          keyboardType="numeric"
        />
      </View>

      {/* Economy/first-class switch */}
      <View style={[styles.switch, { marginBottom: 10 } ]}>
        <Text style={mIsFirstClass ? styles.textInactive : styles.textActive}>Economy</Text>
        <Switch
          trackColor={{ false: '#cccccc', true: 'skyblue' }}
          thumbColor={mIsFirstClass ? '#ffd616' : '#837970'}
          ios_backgroundColor="#3e3e3e"
          onValueChange={toggleFirstClassSwitch}
          value={mIsFirstClass}
          style={{ marginLeft: 10, marginRight: 10 }}
        />
        <Text style={mIsFirstClass ? styles.textActive : styles.textInactive}>Business/First Class</Text>
      </View>

      {/* cheapest/earliest switch */}
      <View style={styles.switch}>
        <Text style={mIsEarlyFlight ? styles.textInactive : styles.textActive}>Lowest price</Text>
        <Switch
          trackColor={{ false: '#cccccc', true: 'skyblue' }}
          thumbColor={mIsEarlyFlight ? '#ffd616' : '#837970'}
          ios_backgroundColor="#3e3e3e"
          onValueChange={toggleEarlyFlightSwitch}
          value={mIsEarlyFlight}
          style={{ marginLeft: 10, marginRight: 10 }}
        />
        <Text style={mIsEarlyFlight ? styles.textActive : styles.textInactive}>First available flight</Text>
      </View>

      {/* Search button */}
      <View style={styles.searchButton}>
        <Button
          ref={mSearchButtonRef}
          title={'Search'}
          onPress={() => {
            // prepare main view
            Keyboard.dismiss();
            if (mMainScrollViewRef.current) mMainScrollViewRef.current.scrollToEnd({ animated: true });

            setIsEmptyFromAirport(mFromAirport.length == 0);
            setIsEmptyToAirport(mToAirport.length == 0);

            setIsSearchPerformed(false);
            setIsFlightsResultsVisible(false);
            setFlightsResultSortedIds([]);
            setFlightsResult([]);

            // stop search if we have validation issues
            if ((mFromAirport.length == 0) || (mToAirport.length == 0)) { return; }

            if (mMainScrollViewRef.current) mMainScrollViewRef.current.measure((x: any, y: any, width: any, height: number, pagex: any, pagey: any) => {
              setSearchScrollPos((height / 3) * 2);
            });
            setIsSearchingFlights(true);

            // prepare main containers
            var searchResults = new Map();
            var searchResultsSorted = [];
            var flightsPerPrice = new Map();
            var flightStops = new Map();
            var startTime = (new Date()).valueOf();

            // each search actually performs two searches:
            // - economy: economy and premium economy
            // - business: business and first class
            fetch({
              // ...
              })
              .catch((error) => {
                setFlightsResultSortedIds([]);
                setFlightsResultVisible([]);
                setFlightsResult([error]);
                setIsSearchingFlights(false);
                setIsFlightsResultsVisible(true);
                setIsSearchPerformed(true);
                console.log(error);
              })
              .finally(() => {
              })
          }
          } />
      </View>

      {/* Progress indicator */}
      {searchProgress()}

      {/* Flights */}
      {flightsResult()}
    </ScrollView>
  );
}

const MainView = () => {
  return RenderMainView();
}

// ---------- STYLES -------------------------------------------------------------------------------

const styles = StyleSheet.create({
  scrollView: {
    backgroundColor: '#e7f5fe',
    paddingTop: 60,
  },
  logoView: {
    alignItems: 'center',
  },
  logo: {
    width: 140,
    height: 140,
    marginTop: -14,
    marginBottom: -14,
    resizeMode: 'contain',
  },
  appTitle: {
    fontSize: 30,
    textAlign: 'center',
  },
  appSubTitle: {
    fontSize: 14,
    textAlign: 'center',
    marginBottom: 10,
  },
  inputView: {
    flex: 1,
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: '#ffffff',
    margin: 12,
    borderWidth: 1,
  },
  inputIcon: {
    marginLeft: 4,
    marginRight: 4,
    color: '#cccccc',
  },
  autocompleteInput: {
    flex: 1,
    borderWidth: 0,
  },
  autocompleteInputList: {
    marginLeft: 12,
    marginRight: 12,
    marginBottom: 12,
    marginTop: -12,
  },
  input: {
    flex: 1,
    borderWidth: 0,
    height: 38,
  },
  suggestedText: {
    fontSize: 15,
    marginLeft: 6,
    marginTop: 2,
    marginBottom: 2,
  },
  switch: {
    flexDirection: 'row',
    alignItems: 'center',
    marginLeft: 12,
  },
  textActive: {
    color: '#000000',
  },
  textInactive: {
    color: '#808080',
  },
  searchButton: {
    margin: 12,
  },
  searchButtonLoading: {
    marginTop: 6,
    marginBottom: 12,
  },
  validationErrMessage: {
    fontSize: 16,
    marginLeft: 12,
    marginRight: 12,
    marginBottom: 12,
    padding: 6,
    borderRadius: 2,
    elevation: 3,

    color: '#f97534',
    backgroundColor: 'white',
  },
  flightResultsList: {
    marginLeft: 12,
    marginRight: 12,
    marginBottom: 12,
    marginTop: 0,
  },
  flightResultsListSmall: {
    marginLeft: 6,
    marginRight: 6,
  },
  flightResultCard: {
    marginTop: 6,
    marginBottom: 6,
    paddingLeft: 6,
    paddingRight: 6,
    paddingTop: 4,
    paddingBottom: 4,

    backgroundColor: 'white',
    borderRadius: 2,
    elevation: 2,
  },
  flightsAvailableTimeView: {
    flexDirection: 'row',
    justifyContent: 'space-between',
  },
  flightTimesText: {
    fontSize: 16,
  },
  flightTimesTextSmall: {
    fontSize: 12,
  },
  flightTimesTextDetails: {
    textAlign: 'right',
    textAlignVertical: 'bottom',
  },
  flightPrice: {
    fontSize: 18,
    fontWeight: 'bold',
    marginTop: 6,
    textAlign: 'right',
  },
  rawResultsView: {
    backgroundColor: 'white',
    borderWidth: 1,
    marginLeft: 12,
    marginRight: 12,
    marginBottom: 12,
  },
  iconStyle: {
    alignSelf: 'center',
    color: 'deepskyblue',
  },
});

export default MainView;