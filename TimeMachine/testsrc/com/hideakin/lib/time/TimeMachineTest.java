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
        testDate(2001, 12, 31);
        testDate(2050, 4, 1);
        TimeMachine.reset();
        Date d9 = new Date();
        System.out.println("9=" + d9);
    }

    @Test
    public void testCalendar() {
        Date d0 = new Date();
        System.out.println("0=" + d0);
        testCalendar(2001, 12, 31);
        testCalendar(2050, 4, 1);
        TimeMachine.reset();
        Date d9 = new Date();
        System.out.println("9=" + d9);
    }

    @Test
    public void testLocalDateTime() {
        Date d0 = new Date();
        System.out.println("0=" + d0);
        testLocalDateTime(2001, 12, 31);
        testLocalDateTime(2050, 4, 1);
        TimeMachine.reset();
        Date d9 = new Date();
        System.out.println("9=" + d9);
    }

    @Test
    public void testZonedDateTime() {
        Date d0 = new Date();
        System.out.println("0=" + d0);
        testZonedDateTime(2001, 12, 31);
        testZonedDateTime(2050, 4, 1);
        TimeMachine.reset();
        Date d9 = new Date();
        System.out.println("9=" + d9);
    }

    private void testDate(int year, int month, int day) {
        TimeMachine.setDate(year, month, day);
        Date d = new Date();
        System.out.println("Date=" + d);
        Calendar cal = Calendar.getInstance();
        cal.setTime(d);
        assertEquals(year,  cal.get(Calendar.YEAR));
        assertEquals(month, cal.get(Calendar.MONTH) + 1);
        assertEquals(day, cal.get(Calendar.DAY_OF_MONTH));
    }

    private void testCalendar(int year, int month, int day) {
        TimeMachine.setDate(year, month, day);
        Calendar cal = Calendar.getInstance();
        Date d = new Date();
        d.setTime(cal.getTimeInMillis());
        System.out.println("Calendar=" + d);
        assertEquals(year, cal.get(Calendar.YEAR));
        assertEquals(month, cal.get(Calendar.MONTH) + 1);
        assertEquals(day, cal.get(Calendar.DAY_OF_MONTH));
    }

    private void testLocalDateTime(int year, int month, int day) {
        TimeMachine.setDate(year, month, day);
        LocalDateTime dt = LocalDateTime.now();
        System.out.println("LocalDateTime=" + dt);
        assertEquals(year, dt.getYear());
        assertEquals(month, dt.getMonthValue());
        assertEquals(day, dt.getDayOfMonth());
    }

    private void testZonedDateTime(int year, int month, int day) {
        TimeMachine.setDate(year, month, day);
        ZonedDateTime dt = ZonedDateTime.now();
        System.out.println("ZonedDateTime=" + dt);
        assertEquals(year, dt.getYear());
        assertEquals(month, dt.getMonthValue());
        assertEquals(day, dt.getDayOfMonth());
    }

}
