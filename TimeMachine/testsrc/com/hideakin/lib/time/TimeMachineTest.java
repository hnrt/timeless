package com.hideakin.lib.time;

import static org.junit.Assert.*;

import java.time.LocalDateTime;
import java.time.ZonedDateTime;
import java.util.Calendar;
import java.util.Date;

import org.junit.Test;

public class TimeMachineTest {

    @Test
    public void testDate() {
        Date d0 = new Date();
        System.out.println("0=" + d0);
        TimeMachine tm = new TimeMachine();
        testDate(tm, 2001, 12, 31);
        testDate(tm, 2050, 4, 1);
        tm.reset();
        Date d9 = new Date();
        System.out.println("9=" + d9);
    }

    @Test
    public void testCalendar() {
        Date d0 = new Date();
        System.out.println("0=" + d0);
        TimeMachine tm = new TimeMachine();
        testCalendar(tm, 2001, 12, 31);
        testCalendar(tm, 2050, 4, 1);
        tm.reset();
        Date d9 = new Date();
        System.out.println("9=" + d9);
    }

    @Test
    public void testLocalDateTime() {
        Date d0 = new Date();
        System.out.println("0=" + d0);
        TimeMachine tm = new TimeMachine();
        testLocalDateTime(tm, 2001, 12, 31);
        testLocalDateTime(tm, 2050, 4, 1);
        tm.reset();
        Date d9 = new Date();
        System.out.println("9=" + d9);
    }

    @Test
    public void testZonedDateTime() {
        Date d0 = new Date();
        System.out.println("0=" + d0);
        TimeMachine tm = new TimeMachine();
        testZonedDateTime(tm, 2001, 12, 31);
        testZonedDateTime(tm, 2050, 4, 1);
        tm.reset();
        Date d9 = new Date();
        System.out.println("9=" + d9);
    }

    private void testDate(TimeMachine tm, int year, int month, int day) {
        tm.setDate(year, month, day);
        Date d = new Date();
        System.out.println("Date=" + d);
        Calendar cal = Calendar.getInstance();
        cal.setTime(d);
        assertEquals(year,  cal.get(Calendar.YEAR));
        assertEquals(month, cal.get(Calendar.MONTH) + 1);
        assertEquals(day, cal.get(Calendar.DAY_OF_MONTH));
    }

    private void testCalendar(TimeMachine tm, int year, int month, int day) {
        tm.setDate(year, month, day);
        Calendar cal = Calendar.getInstance();
        Date d = new Date();
        d.setTime(cal.getTimeInMillis());
        System.out.println("Calendar=" + d);
        assertEquals(year, cal.get(Calendar.YEAR));
        assertEquals(month, cal.get(Calendar.MONTH) + 1);
        assertEquals(day, cal.get(Calendar.DAY_OF_MONTH));
    }

    private void testLocalDateTime(TimeMachine tm, int year, int month, int day) {
        tm.setDate(year, month, day);
        LocalDateTime dt = LocalDateTime.now();
        System.out.println("LocalDateTime=" + dt);
        assertEquals(year, dt.getYear());
        assertEquals(month, dt.getMonthValue());
        assertEquals(day, dt.getDayOfMonth());
    }

    private void testZonedDateTime(TimeMachine tm, int year, int month, int day) {
        tm.setDate(year, month, day);
        ZonedDateTime dt = ZonedDateTime.now();
        System.out.println("ZonedDateTime=" + dt);
        assertEquals(year, dt.getYear());
        assertEquals(month, dt.getMonthValue());
        assertEquals(day, dt.getDayOfMonth());
    }

}
